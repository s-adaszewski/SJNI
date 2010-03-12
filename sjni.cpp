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
    * Neither the name of Stanislaw Adaszewski nor the
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

#if defined(_SJNI_DETECT_LEAKS_)
int _sjni_total_ref_count;
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

sjniFld::sjniFld(JNIEnv *aEnv, /* const char *aClsName, */ jclass ownerCls, const char *name, const char *sig): obj(0), clsName(0), cls(0)
{
	env = aEnv;
	if (sig[0] == 'L')
	{
		clsName = strdup(sig + 1);
		clsName[strlen(clsName)-1] = 0;
		sjniCls clz(env, clsName);
		cls = (jclass) env->NewLocalRef(clz.jcls()); _SJNI_INC_REF_COUNT2(cls);
	}
	// cls = aCls;
	fieldID = env->GetFieldID(ownerCls, name, sig); _SJNI_FNFE_Z(fieldID, name, sig, ownerCls);
}

sjniFld::sjniFld(JNIEnv *aEnv, /* const char *aClassName, */ jclass ownerCls, jobject aObj, const char *name, const char *sig): clsName(0), cls(0)
{
	env = aEnv;
	if (sig[0] == 'L')
	{
		clsName = strdup(sig + 1);
		clsName[strlen(clsName)-1] = 0;
		sjniCls clz(env, clsName);
		cls = (jclass) env->NewLocalRef(clz.jcls()); _SJNI_INC_REF_COUNT2(cls);
	}
	// cls = aCls;
	fieldID = env->GetFieldID(ownerCls, name, sig); _SJNI_FNFE_Z(fieldID, name, sig, ownerCls);
	obj = aObj;
}

sjniSFld::sjniSFld(JNIEnv *aEnv, /* const char *aClsName, */ jclass aCls, const char *name, const char *sig): clsName(0), cls(0), fieldCls(0) // obj(0)
{
	env = aEnv;
	cls = aCls;
	if (sig[0] == 'L')
	{
		clsName = strdup(sig + 1);
		clsName[strlen(clsName) - 1] = 0;
		// cls = aCls;
		sjniCls clz(env, clsName);
		fieldCls = (jclass) env->NewLocalRef(clz.jcls()); _SJNI_INC_REF_COUNT2(fieldCls);
	}
	cls = (jclass) env->NewLocalRef(aCls); _SJNI_INC_REF_COUNT2(cls);
	fieldID = env->GetStaticFieldID(cls, name, sig); _SJNI_FNFE_Z(fieldID, name, sig, cls);
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
	jobject aryObj = env->CallObjectMethodA(obj, methodID, args); _SJNI_CHECK_JVM_EXC; _SJNI_INC_REF_COUNT2(aryObj);
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
	jobject aryObj = env->CallStaticObjectMethodA(cls, methodID, args); _SJNI_CHECK_JVM_EXC; _SJNI_INC_REF_COUNT2(aryObj);
	aAry.receiveObject(env, aryObj);
}

void sjniEnv::jfree(void *mem)
{
#if defined(_WIN32)
	static int been_here(0);
	static HANDLE hMsvcrDll;
	static void (*free_from_jvm_msvcr)(void*);
	if (!been_here)
	{
		hMsvcrDll = LoadLibrary(_SJNI_JVM_MSVCR_DLL_);
		free_from_jvm_msvcr = (void(*)(void*)) GetProcAddress(hMsvcrDll, "free");
	}
	free_from_jvm_msvcr(mem);
#else
	free(mem);
#endif // _WIN32
}

#if defined(_SJNI_UNIT_1_)
int main(int argc, char *argv[])
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
}
#endif // _SJNI_UNIT_1_

#if defined(_SJNI_UNIT_2_)
int main(int argc, char *argv)
{
	sjniEnv e(JNI_VERSION_1_6);
	printf("vm = 0x%08X env = 0x%08X\n", e.jvm(), e.jenv());
	sjniPkg p = e.pkg("javax/swing");
	sjniCls JFrame = p.cls("JFrame");
	sjniObj frame = JFrame.nnew("()V"); // p.nnew("JFrame", "()V");
	(frame << "setSize" << 300 << 200).callV();
	sjniStr title(e.jenv(), "Test"); // = e.str("Test");
	printf("title clsName = %s\n", title.getClsName());
	(frame << "setTitle" << title).callV(); // return 0;
	int EXIT_ON_CLOSE = (jint) JFrame.sfld("EXIT_ON_CLOSE", "I"); // .getI();
	printf("EXIT_ON_CLOSE = %d\n", EXIT_ON_CLOSE);
	(frame << "setDefaultCloseOperation" << EXIT_ON_CLOSE).callV();
	// sleep(1000);
	(frame << "setVisible" << jtrue).callV();
	// sleep(1000);
	while (1)
	{
		sleep(1);
	}
}
#endif // _SJNI_UNIT_2_

#if defined(_SJNI_UNIT_3_)
int main(int argc, char *argv[])
{
	sjniEnv e;
	try
	{
		// sjniCls test1 = e.cls("there/is/no/such/class");
		sjniCls test2 = e.cls("java/lang/System");
		// (test2.sCall("noSuchMethod") << (jint) 1).callV();
		sjniSFld fld = test2.sfld("out", "Ljava/io/PrintStream;");
		test2.fld("noSuchField", "I") << (jint) 1;
	} catch (sjniException e)
	{
		printf("Exception caught:\n\t%s\n", e.msg());
	}
	printf("_sjni_total_ref_count = %d\n", _sjni_total_ref_count);
}
#endif // _SJNI_UNIT_3_

#if defined(_SJNI_UNIT_4_)
int main(int argc, char *argv[])
{
	sjniEnv e;
	try
	{
		sjniCls System = e.cls("java/lang/System");
		sjniObj out = sjniObj(System.sfld("out", "Ljava/io/PrintStream;"));
		// (out << "println" << sjniObj(e.jenv(), "java/lang/String", (jobject) 0)).callV();
		sjniObj exc = e.nnew("java/lang/Exception", "(Ljava/lang/String;)V", e.str("Whazaaa").jobj());
		e.jenv()->Throw((jthrowable) exc.jobj());
		sjniExceptionHelper::throwIfJavaExceptionOccured(e.jenv());
	} catch (sjniException e)
	{
		printf("Exception caught:\n\t%s\n", e.msg());
	}
	printf("_sjni_total_ref_count = %d\n", _sjni_total_ref_count);
}
#endif // _SJNI_UNIT_4_
