#include "sjni.h"

#include <unistd.h>

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
		clsName = strdup(sig + 1);
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
		clsName = strdup(sig + 1);
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
		clsName = strdup(sig + 1);
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

	jobject s = e.jenv()->NewStringUTF("DUPA CIPA CHUJ!");
	e.jenv()->CallVoidMethod(out.jobj(), println.jmet(), s);

	sjniStr s1(e.jenv(), "Dupa kurwa chuj");
	sjniStr s2(s1);
	printf("s2.jobj() == 0x%08X\n", s2.jobj());
	(out << "println" << s1).callV();
	(out << "flush").callV();

	printf("done calling stufff...\n");

	e.destroy();
	_exit(0);
} */

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
	(frame << "setTitle" << title).callV();
	int EXIT_ON_CLOSE = frame.sfld("EXIT_ON_CLOSE", "I").getI();
	printf("EXIT_ON_CLOSE = %d\n", EXIT_ON_CLOSE);
	(frame << "setDefaultCloseOperation" << EXIT_ON_CLOSE).callV();
	// sleep(1000);
	(frame << "setVisible" << (jboolean) true).callV();
	sleep(1000);
	while (1)
	{
		sleep(1);
	}
}
