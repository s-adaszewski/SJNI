/*

Copyright (c) 2010, Stanislaw Adaszewski (s.adaszewski@aster.pl)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Stanislaw Adaszewski nor the
      names of any contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL STANISLAW ADASZEWSKI BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "sjni.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define sleep(x) Sleep(x)
#else
#include <unistd.h>
#endif

/* sjniCall sjniObj::operator<< (const char *met)
{
	return sjniCall(env, cls, obj, met);
}

sjniCall sjniObj::sCall (const char *met)
{
}

// sjniCall operator<< (const sjniMet&);
sjniCall sjniObj::operator<< (jmethodID metID)
{
	return sjniCall(env, obj, metID);
} */

sjniFld::sjniFld(JNIEnv *aEnv, /* const char *aClsName, jclass aCls, */ const char *name, const char *sig): obj(0), clsName(0), cls(0)
{
	env = aEnv;
	if (sig[0] == 'L')
	{
		clsName = _strdup(sig + 1);
		clsName[strlen(clsName)-1] = 0;
		sjniCls clz(env, clsName);
		cls = (jclass) env->NewLocalRef(clz.jcls());
	}
	// cls = aCls;
	fieldID = env->GetFieldID(cls, name, sig);
}

sjniFld::sjniFld(JNIEnv *aEnv, /* const char *aClassName,  jclass aCls, */ jobject aObj, const char *name, const char *sig): clsName(0), cls(0)
{
	env = aEnv;
	if (sig[0] == 'L')
	{
		clsName = _strdup(sig + 1);
		clsName[strlen(clsName)-1] = 0;
		sjniCls clz(env, clsName);
		cls = (jclass) env->NewLocalRef(clz.jcls());
	}
	// cls = aCls;
	fieldID = env->GetFieldID(cls, name, sig);
	obj = aObj;
}

sjniSFld::sjniSFld(JNIEnv *aEnv, /* const char *aClsName, */ jclass aCls, const char *name, const char *sig): clsName(0), cls(0) // obj(0)
{
	env = aEnv;
	cls = aCls;
	if (sig[0] == 'L')
	{
		clsName = _strdup(sig + 1);
		clsName[strlen(clsName) - 1] = 0;
		// cls = aCls;
		sjniCls clz(env, clsName);
		fieldCls = (jclass) env->NewLocalRef(clz.jcls());
	}
	cls = (jclass) env->NewLocalRef(aCls);
	fieldID = env->GetStaticFieldID(cls, name, sig);
}

sjniCall& sjniCall::operator<< (const sjniObj &aObj)
{
	append2sig("L");
	append2sig(aObj.getClsName());
	append2sig(";");
	append2args()->l = aObj.jobj();
	return *this;
}

sjniCall& sjniCall::operator<< (const sjniAry &aAry)
{
	append2sig(aAry.sig());
	append2args()->l = aAry.jobj();
	return *this;
}

void sjniCall::callO(const char *clz, sjniObj &aObj)
{
	jobject obj = callO(clz);
	aObj = sjniObj(env, clz, obj);
}

void sjniCall::callA(sjniAry &aAry)
{
	prepMethodID(aAry.sig()); // "[", aAry.sig());
	jobject aryObj = env->CallObjectMethodA(obj, methodID, args);
	aAry.receiveObject(env, aryObj);
}

void sjniSCall::callO(const char *clz, sjniObj &aObj)
{
	jobject obj = callO(clz);
	aObj = sjniObj(env, clz, obj);
}

void sjniSCall::callA(sjniAry &aAry)
{
	prepMethodID(aAry.sig()); // "[", aAry.sig());
	jobject aryObj = env->CallStaticObjectMethodA(cls, methodID, args);
	aAry.receiveObject(env, aryObj);
}

/* int main(int argc, char *argv[])
{
	sjniEnv e;
	sjniCls System = e.cls("java/lang/System");
	sjniCls System2(System);

	jlong cur_time = (System.sCall("currentTimeMillis")).callL();
	printf("cur_time = %d\n", cur_time);

	sjniCls PrintStream = e.cls("java/io/PrintStream");
	sjniMet println = PrintStream.met("println", "(Ljava/lang/String;)V");
	printf("println methodID = 0x%08X\n", println.jmet());

	sjniObj out = sjniObj(System.sfld("out", "Ljava/io/PrintStream;"));

	jobject s = e.jenv()->NewStringUTF("foo bar bar!");
	e.jenv()->CallVoidMethod(out.jobj(), println.jmet(), s);

	sjniStr s1(e.jenv(), "foo foo bar");
	sjniStr s2(s1);
	printf("s2.jobj() == 0x%08X\n", s2.jobj());
	(out << "println" << s1).callV();
	(out << "flush").callV();

	sjniAry charAry(e.ary(5, "C"));
	charAry.set(0, (jchar) '1');
	charAry.set(1, (jchar) '1');
	charAry.set(2, (jchar) '2');
	charAry.set(3, (jchar) '3');
	charAry.set(4, (jchar) '5');
	(out << "println" << charAry).callV();
	(out << "flush").callV();

	printf("charAry.len() == %d\n", charAry.len());

#define C (jchar)

	charAry.setCurIdx(0);
	charAry << C'7' << C'8' << C'9' << C'a' << C'b';
	(out << "println" << charAry).callV();

	printf("done calling stufff...\n");

	e.destroy();
	_exit(0);
} */

/* int main(int argc, char *argv)
{
	sjniEnv e(JNI_VERSION_1_6);
	printf("vm = 0x%08X env = 0x%08X\n", e.jvm(), e.jenv());
	sjniPkg p = e.pkg("javax/swing");
	sjniCls JFrame = p.cls("JFrame");
	sjniObj frame = JFrame.nnew("()V"); // p.nnew("JFrame", "()V");
	(frame << "setSize" << 300 << 200).callV();
	sjniStr title(e.jenv(), "Test"); // = e.str("Test");
	printf("title clsName = %s\n", title.getClsName());
	(frame << "setTitle" << title).callV();
	int EXIT_ON_CLOSE = JFrame.sfld("EXIT_ON_CLOSE", "I"); // .getI();
	printf("EXIT_ON_CLOSE = %d\n", EXIT_ON_CLOSE);
	(frame << "setDefaultCloseOperation" << EXIT_ON_CLOSE).callV();
	// sleep(1000);
	(frame << "setVisible" << jtrue).callV();
	// sleep(1000);
	while (1)
	{
		sleep(1);
	}
} */
