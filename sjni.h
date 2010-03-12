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

#ifndef _SJNI_H_
#define _SJNI_H_

#include <jni.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#ifdef _SJNI_DETECT_LEAKS_
extern int _sjni_total_ref_count;
#define _SJNI_INC_REF_COUNT _sjni_total_ref_count++
#define _SJNI_DEC_REF_COUNT _sjni_total_ref_count--
#define _SJNI_INC_REF_COUNT2(x) { if (x) _sjni_total_ref_count++; }
#else
#define _SJNI_INC_REF_COUNT
#define _SJNI_DEC_REF_COUNT
#define _SJNI_INC_REF_COUNT2(x)
#endif

#ifdef _WIN32
#define strdup(x) _strdup(x)

#if !defined(_SJNI_JVM_MSVCR_DLL_)
#define _SJNI_JVM_MSVCR_DLL_ "msvcr70.dll"
#endif // _SJNI_JVM_MSVCR_DLL_
#endif // _WIN32

#define jtrue ((jboolean) true)
#define jfalse ((jboolean) false)

class sjniMet;
class sjniFld;
class sjniCls;
class sjniCall;
class sjniObj;
class sjniPkg;
class sjniAry;

#ifdef _SJNI_EXCEPTIONS_
class sjniException
{
public:
	sjniException(const char *aMsg)
	{
		iMsg = strdup(aMsg);
	}
	sjniException(const sjniException& other)
	{
		iMsg = strdup(other.iMsg);
	}
	~sjniException()
	{
		free(iMsg);
	}
	sjniException& operator= (const sjniException &other)
	{
		iMsg = strdup(other.iMsg);
		return *this;
	}
	const char* msg() const
	{
		return iMsg;
	}
private:
	char *iMsg;
};

class sjniClassNotFoundException: public sjniException
{
public:
	sjniClassNotFoundException(const char *aMsg): sjniException(aMsg) {}
};

class sjniMethodNotFoundException: public sjniException
{
public:
	sjniMethodNotFoundException(const char *aMsg): sjniException(aMsg) {}
};

class sjniFieldNotFoundException: public sjniException
{
public:
	sjniFieldNotFoundException(const char *aMsg): sjniException(aMsg) {}
};

#define _SJNI_CNFE_Z(c, name)\
if (c == 0) {\
	char excMsgBuf[256];\
	snprintf(excMsgBuf, 256, "Unable to find Java class %s", name);\
	throw sjniClassNotFoundException(excMsgBuf);\
}
#define _SJNI_MNFE_Z(m, name, sig, cls)\
if (m == 0) {\
	char excMsgBuf[256];\
	snprintf(excMsgBuf, 256, "Unable to find method %s %s in class 0x%08X", name, sig, cls);\
	throw sjniMethodNotFoundException(excMsgBuf);\
}
#define _SJNI_FNFE_Z(f, name, sig, cls)\
if (f == 0) {\
	char excMsgBuf[256];\
	snprintf(excMsgBuf, 256, "Unable to find field %s %s in class 0x%08X", name, sig, cls);\
	throw sjniFieldNotFoundException(excMsgBuf);\
}
#else
#define _SJNI_CNFE_Z(c, name) { printf("Unable to find Java class ##name\n"); exit(-1); }
#define _SJNI_MNFE_Z(m, name, sig, cname) { printf("Unable to find method ##name ##sig in class ##cname\n"); exit(-1); }
#define _SJNI_FNFE_Z(f, name, sig, cname) { printf("Unable to find field ##name ##sig in class ##cname\n"); exit(-1); }
#endif // _SJNI_EXCEPTIONS_

#if defined(_SJNI_EXCEPTIONS_) && defined(_SJNI_JAVA_EXCEPTIONS_)
class sjniJavaException: public sjniException
{
public:
	sjniJavaException(const char *aMsg): sjniException(aMsg) {}
};

#define _SJNI_CHECK_JVM_EXC sjniExceptionHelper::throwIfJavaExceptionOccured(env)

class sjniExceptionHelper
{
public:
	static void throwIfJavaExceptionOccured(JNIEnv *aEnv);
};
#else
#define _SJNI_CHECK_JVM_EXC
#endif // _SJNI_EXCEPTIONS_ && _SJNI_JAVA_EXCEPTIONS_

class sjniMet
{
public:
	sjniMet(JNIEnv *env, jclass cls, const char *name, const char *sig)
	{
		methodID = env->GetMethodID(cls, name, sig); _SJNI_MNFE_Z(methodID, name, sig, cls);
	}

	jmethodID jmet() { return methodID; }

private:
	sjniMet& operator=(const sjniMet&) { return *this; }

private:
	jmethodID methodID;
};

class sjniFld
{
public:
	sjniFld(JNIEnv *aEnv, /* const char *aClsName, */ jclass ownerCls, const char *name, const char *sig);
	
	sjniFld(JNIEnv *aEnv, /* const char *aClassName, */ jclass ownerCls, jobject aObj, const char *name, const char *sig);
	
	~sjniFld()
	{
		if (cls) { env->DeleteLocalRef(cls); _SJNI_DEC_REF_COUNT; }
		free(clsName);
	}

	sjniFld& operator>> (jboolean &z) { z = env->GetBooleanField(obj, fieldID); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniFld& operator>> (jbyte &b) { b = env->GetByteField(obj, fieldID); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniFld& operator>> (jchar &c) { c = env->GetCharField(obj, fieldID); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniFld& operator>> (jshort &s) { s = env->GetShortField(obj, fieldID); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniFld& operator>> (jint &i) { i = env->GetIntField(obj, fieldID); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniFld& operator>> (jlong &l) { l = env->GetLongField(obj, fieldID); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniFld& operator>> (jfloat &f) { f = env->GetFloatField(obj, fieldID); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniFld& operator>> (jdouble &d) { d = env->GetDoubleField(obj, fieldID); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniFld& operator>> (jobject &o) { o = env->GetObjectField(obj, fieldID); _SJNI_CHECK_JVM_EXC; _SJNI_INC_REF_COUNT2(o); return *this; }
	// sjniFld& operator>> (sjniObj &o) { o = sjniObj(env->GetObjectField(obj, fieldID)); return *this; }

	sjniFld& operator<< (jboolean z) { env->SetBooleanField(obj, fieldID, z); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniFld& operator<< (jbyte b) { env->SetByteField(obj, fieldID, b); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniFld& operator<< (jchar c) { env->SetCharField(obj, fieldID, c); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniFld& operator<< (jshort s) { env->SetShortField(obj, fieldID, s); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniFld& operator<< (jint i) { env->SetIntField(obj, fieldID, i); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniFld& operator<< (jlong l) { env->SetLongField(obj, fieldID, l); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniFld& operator<< (jfloat f) { env->SetFloatField(obj, fieldID, f); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniFld& operator<< (jdouble d) { env->SetDoubleField(obj, fieldID, d); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniFld& operator<< (jobject o) { env->SetObjectField(obj, fieldID, o); _SJNI_CHECK_JVM_EXC; return *this; }
	// sjniFld& operator<< (const sjniObj &o) { env->SetObjectField(obj, fieldID, o.jobj()); return *this; }

	jboolean getZ() { jboolean tmp = env->GetBooleanField(obj, fieldID); _SJNI_CHECK_JVM_EXC; return tmp; }
	jbyte getB() { jbyte tmp = env->GetByteField(obj, fieldID); _SJNI_CHECK_JVM_EXC; return tmp; }
	jchar getC() { jchar tmp = env->GetCharField(obj, fieldID); _SJNI_CHECK_JVM_EXC; return tmp; }
	jshort getS() { jshort tmp = env->GetShortField(obj, fieldID); _SJNI_CHECK_JVM_EXC; return tmp; }
	jint getI() { jint tmp = env->GetIntField(obj, fieldID); _SJNI_CHECK_JVM_EXC; return tmp; }
	jlong getL() { jlong tmp = env->GetLongField(obj, fieldID); _SJNI_CHECK_JVM_EXC; return tmp; }
	jfloat getF() { jfloat tmp = env->GetFloatField(obj, fieldID); _SJNI_CHECK_JVM_EXC; return tmp; }
	jdouble getD() { jdouble tmp = env->GetDoubleField(obj, fieldID); _SJNI_CHECK_JVM_EXC; return tmp; }
	jobject getO() const { _SJNI_INC_REF_COUNT; jobject tmp = env->GetObjectField(obj, fieldID); _SJNI_CHECK_JVM_EXC; return tmp; }
	// sjniObj getO() { return sjniObj(env->GetObjectField(obj, fieldID)); }

	operator jboolean() { return getZ(); }
	operator jbyte() { return getB(); }
	operator jchar() { return getC(); }
	operator jshort() { return getS(); }
	operator jint() { return getI(); }
	operator jlong() { return getL(); }
	operator jfloat() { return getF(); }
	operator jdouble() { return getD(); }
	operator jobject() { return getO(); }

	sjniFld& operator= (jboolean z) { (*this) << z; return *this; }
	sjniFld& operator= (jbyte b) { (*this) << b; return *this; }
	sjniFld& operator= (jchar c) { (*this) << c; return *this; }
	sjniFld& operator= (jshort s) { (*this) << s; return *this; }
	sjniFld& operator= (jint i) { (*this) << i; return *this; }
	sjniFld& operator= (jlong l) { (*this) << l; return *this; }
	sjniFld& operator= (jfloat f) { (*this) << f; return *this; }
	sjniFld& operator= (jdouble d) { (*this) << d; return *this; }
	sjniFld& operator= (jobject o) { (*this) << o; return *this; }

	JNIEnv* jenv() const { return env; }
	const char* getClassName() const { return clsName; }
	jclass jcls() const { return cls; }

private:
	sjniFld& operator= (const sjniFld&) { return *this; }

private:
	JNIEnv *env;
	char *clsName;
	jclass cls;
	jobject obj;
	jfieldID fieldID;
};

class sjniSFld
{
public:
	sjniSFld(JNIEnv *aEnv, /* const char *aClsName, */ jclass aCls, const char *name, const char *sig);

	~sjniSFld()
	{
		free(clsName);
		// env->DeleteLocalRef(cls);
		if (fieldCls)
		{
			env->DeleteLocalRef(fieldCls); _SJNI_DEC_REF_COUNT;
		}
		if (cls)
		{
			env->DeleteLocalRef(cls); _SJNI_DEC_REF_COUNT;
		}
	}

	/* sjniSFld(JNIEnv *aEnv, jclass cls, jobject aObj, const char *name, const char *sig)
	{
		env = aEnv;
		fieldID = env->GetFieldID(cls, name, sig);
		obj = aObj;
	} */

	sjniSFld& operator>> (jboolean &z) { z = env->GetStaticBooleanField(cls, fieldID); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniSFld& operator>> (jbyte &b) { b = env->GetStaticByteField(cls, fieldID); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniSFld& operator>> (jchar &c) { c = env->GetStaticCharField(cls, fieldID); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniSFld& operator>> (jshort &s) { s = env->GetStaticShortField(cls, fieldID); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniSFld& operator>> (jint &i) { i = env->GetStaticIntField(cls, fieldID); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniSFld& operator>> (jlong &l) { l = env->GetStaticLongField(cls, fieldID); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniSFld& operator>> (jfloat &f) { f = env->GetStaticFloatField(cls, fieldID); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniSFld& operator>> (jdouble &d) { d = env->GetStaticDoubleField(cls, fieldID); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniSFld& operator>> (jobject &o) { o = env->GetStaticObjectField(cls, fieldID); _SJNI_CHECK_JVM_EXC; _SJNI_INC_REF_COUNT2(o); return *this; }
	// sjniFld& operator>> (sjniObj &o) { o = sjniObj(env->GetObjectField(obj, fieldID)); return *this; }

	sjniSFld& operator<< (jboolean z) { env->SetStaticBooleanField(cls, fieldID, z); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniSFld& operator<< (jbyte b) { env->SetStaticByteField(cls, fieldID, b); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniSFld& operator<< (jchar c) { env->SetStaticCharField(cls, fieldID, c); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniSFld& operator<< (jshort s) { env->SetStaticShortField(cls, fieldID, s); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniSFld& operator<< (jint i) { env->SetStaticIntField(cls, fieldID, i); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniSFld& operator<< (jlong l) { env->SetStaticLongField(cls, fieldID, l); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniSFld& operator<< (jfloat f) { env->SetStaticFloatField(cls, fieldID, f); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniSFld& operator<< (jdouble d) { env->SetStaticDoubleField(cls, fieldID, d); _SJNI_CHECK_JVM_EXC; return *this; }
	sjniSFld& operator<< (jobject o) { env->SetStaticObjectField(cls, fieldID, o); _SJNI_CHECK_JVM_EXC; return *this; }
	// sjniFld& operator<< (const sjniObj &o) { env->SetObjectField(obj, fieldID, o.jobj()); return *this; }

	jboolean getZ() { jboolean tmp = env->GetStaticBooleanField(cls, fieldID); _SJNI_CHECK_JVM_EXC; return tmp; }
	jbyte getB() { jbyte tmp = env->GetStaticByteField(cls, fieldID); _SJNI_CHECK_JVM_EXC; return tmp; }
	jchar getC() { jchar tmp = env->GetStaticCharField(cls, fieldID); _SJNI_CHECK_JVM_EXC; return tmp; }
	jshort getS() { jshort tmp = env->GetStaticShortField(cls, fieldID); _SJNI_CHECK_JVM_EXC; return tmp; }
	jint getI() { jint tmp = env->GetStaticIntField(cls, fieldID); _SJNI_CHECK_JVM_EXC; return tmp; }
	jlong getL() { jlong tmp = env->GetStaticLongField(cls, fieldID); _SJNI_CHECK_JVM_EXC; return tmp; }
	jfloat getF() { jfloat tmp = env->GetStaticFloatField(cls, fieldID); _SJNI_CHECK_JVM_EXC; return tmp; }
	jdouble getD() { jdouble tmp = env->GetStaticDoubleField(cls, fieldID); _SJNI_CHECK_JVM_EXC; return tmp; }
	jobject getO() const { jobject o = env->GetStaticObjectField(cls, fieldID); _SJNI_CHECK_JVM_EXC; _SJNI_INC_REF_COUNT2(o); return o; }
	// sjniObj getO() { return sjniObj(env->GetObjectField(obj, fieldID)); }

	operator jboolean() { return getZ(); }
	operator jbyte() { return getB(); }
	operator jchar() { return getC(); }
	operator jshort() { return getS(); }
	operator jint() { return getI(); }
	operator jlong() { return getL(); }
	operator jfloat() { return getF(); }
	operator jdouble() { return getD(); }
	operator jobject() { return getO(); }

	sjniSFld& operator= (jboolean z) { (*this) << z; return *this; }
	sjniSFld& operator= (jbyte b) { (*this) << b; return *this; }
	sjniSFld& operator= (jchar c) { (*this) << c; return *this; }
	sjniSFld& operator= (jshort s) { (*this) << s; return *this; }
	sjniSFld& operator= (jint i) { (*this) << i; return *this; }
	sjniSFld& operator= (jlong l) { (*this) << l; return *this; }
	sjniSFld& operator= (jfloat f) { (*this) << f; return *this; }
	sjniSFld& operator= (jdouble d) { (*this) << d; return *this; }
	sjniSFld& operator= (jobject o) { (*this) << o; return *this; }

	JNIEnv* jenv() const { return env; }
	const char* getClassName() const { return clsName; }
	jclass jcls() const { return fieldCls; }

private:
	sjniSFld& operator=(const sjniSFld&) { return *this; }

private:
	JNIEnv *env;
	char *clsName;
	jclass cls;
	jclass fieldCls;
	jfieldID fieldID;
};

class sjniCall
{
public:
	sjniCall() {}

	// sjniCall(const sjniObj&, const char *met, bool static_);
	sjniCall(JNIEnv *aEnv, jclass aCls, jobject aObj, const char *aMethod): sig(0), len(0), alen(0), args(0), nargs(0), aargs(0), methodID(0)
	{
		env = aEnv;
		// cls = aCls;
		cls = (jclass) env->NewLocalRef(aCls); _SJNI_INC_REF_COUNT;
		// obj = aObj;
		obj = env->NewLocalRef(aObj); _SJNI_INC_REF_COUNT;
		method = aMethod;
		// methodID = env->GetMethodID(cls, 
		append2sig("(");
	}

	sjniCall(JNIEnv *aEnv, jobject aObj, jmethodID metID): method(0), sig(0), len(0), alen(0), args(0), nargs(0), aargs(0), cls(0)
	{
		env = aEnv;
		// obj = aObj;
		obj = env->NewLocalRef(aObj); _SJNI_INC_REF_COUNT;
		methodID = metID;
	}

	~sjniCall()
	{
		if (cls) { env->DeleteLocalRef(cls); _SJNI_DEC_REF_COUNT; }
		if (obj) { env->DeleteLocalRef(obj); _SJNI_DEC_REF_COUNT; }
		free(sig);
		free(args);
	}

	sjniCall& operator<< (jboolean z) { append2sig("Z"); append2args()->z = z; return *this; }
	sjniCall& operator<< (jbyte b) { append2sig("B"); append2args()->b = b; return *this; }
	sjniCall& operator<< (jchar c) { append2sig("C"); append2args()->c = c; return *this; }
	sjniCall& operator<< (jshort s) { append2sig("S"); append2args()->s = s; return *this; }
	sjniCall& operator<< (jint i) { append2sig("I"); append2args()->i = i; return *this; }
	sjniCall& operator<< (jlong l) { append2sig("J"); append2args()->j = l; return *this; }
	sjniCall& operator<< (jfloat f) { append2sig("F"); append2args()->f = f; return *this; } 
	sjniCall& operator<< (jdouble d) { append2sig("D"); append2args()->d = d; return *this; }
	// sjniCall& operator<< (jobject, const char *clz);
	sjniCall& arg(jobject aObj, const char *clz) { append2sig("L"); append2sig(clz); append2sig(";"); append2args()->l = aObj; return *this; }
	sjniCall& operator<< (const sjniObj &aObj);
	sjniCall& operator<< (const sjniAry &aAry);

	virtual void callV() { prepMethodID("V"); env->CallVoidMethodA(obj, methodID, args); _SJNI_CHECK_JVM_EXC; }
	virtual jboolean callZ() { prepMethodID("Z"); jboolean tmp = env->CallBooleanMethodA(obj, methodID, args); _SJNI_CHECK_JVM_EXC; return tmp; }
	virtual jbyte callB() { prepMethodID("B"); jbyte tmp = env->CallByteMethodA(obj, methodID, args); _SJNI_CHECK_JVM_EXC; return tmp; }
	virtual jchar callC() { prepMethodID("C"); jchar tmp = env->CallCharMethodA(obj, methodID, args); _SJNI_CHECK_JVM_EXC; return tmp; }
	virtual jshort callS() { prepMethodID("S"); jshort tmp = env->CallShortMethodA(obj, methodID, args); _SJNI_CHECK_JVM_EXC; return tmp; }
	virtual jint callI() { prepMethodID("I"); jint tmp = env->CallIntMethodA(obj, methodID, args); _SJNI_CHECK_JVM_EXC; return tmp; }
	virtual jlong callL() { prepMethodID("J"); jlong tmp = env->CallLongMethodA(obj, methodID, args); _SJNI_CHECK_JVM_EXC; return tmp; }
	virtual jfloat callF() { prepMethodID("F"); jfloat tmp = env->CallFloatMethodA(obj, methodID, args); _SJNI_CHECK_JVM_EXC; return tmp; }
	virtual jdouble callD() { prepMethodID("D"); jdouble tmp = env->CallDoubleMethodA(obj, methodID, args); _SJNI_CHECK_JVM_EXC; return tmp; }
	virtual jobject callO(const char *clz) { prepMethodID("L", clz); jobject o = env->CallObjectMethodA(obj, methodID, args); _SJNI_CHECK_JVM_EXC; _SJNI_INC_REF_COUNT2(o); return o; }
	virtual void callO(const char *clz, sjniObj &);
	virtual void callA(sjniAry&);
	// sjniObj callO(const char *clz) { prepMethodID("L", clz); env->CallObjectMethod(obj, methodID, args); }

private:
	virtual sjniCall& operator=(const sjniCall&) { return *this; }

protected:
	void append2sig(const char *s)
	{
		if (methodID) return;

		int l = strlen(s);
		if (len + l > alen)
		{
			alen = (len + l) * 2; // + 1;
			sig = (char*) realloc(sig, alen + 1);
			sig[len] = 0;
		}
		strcat(sig, s);
		len += l;
	}

	jvalue* append2args() // jvalue v)
	{
		if (methodID) return &dummy;

		if (nargs == aargs)
		{
			aargs += 4;
			args = (jvalue*) realloc(args, sizeof(jvalue) * aargs);
		}
		// args[nargs] = v;
		nargs++;
		return &args[nargs-1];
	}

	virtual void prepMethodID(const char *type)
	{
		if (!methodID)
		{
			append2sig(")");
			append2sig(type);
			printf("GetMethodID() cls = 0x%08X, method = %s, sig = %s\n", cls, method, sig);
			methodID = env->GetMethodID(cls, method, sig);
			if (!methodID)
			{
				printf("Method %s %s not found\n", method, sig);
			}
			_SJNI_MNFE_Z(methodID, method, sig, cls);
		}

		printf("nargs = %d aargs = %d\n", nargs, aargs);
		for (int i = 0; i < nargs; i++)
		{
			printf("0x%08X\n", args[i].l);
		}
	}

	virtual void prepMethodID(const char *type, const char *clz)
	{
		if (!methodID)
		{
			append2sig(")");
			append2sig(type);
			append2sig(clz);
			append2sig(";");
			methodID = env->GetMethodID(cls, method, sig);
			if (!methodID)
			{
				printf("Method %s %s not found\n", method, sig);
			}
			_SJNI_MNFE_Z(methodID, method, sig, cls);
		}
	}

protected:
	JNIEnv *env;
	jclass cls;
	jobject obj;
	const char *method;
	jmethodID methodID;
	char *sig;
	int len, alen;
	jvalue *args;
	int nargs, aargs;
	jvalue dummy;
};

class sjniSCall: public sjniCall
{
public:
	sjniSCall(JNIEnv *aEnv, jclass aCls, const char *aMethod)
	{
		sig = 0, len = 0, alen = 0, args = 0, nargs = 0, aargs = 0, methodID = 0, obj = 0;

		env = aEnv;
		// cls = aCls;
		cls = (jclass) env->NewLocalRef(aCls); _SJNI_INC_REF_COUNT;
		// obj = aObj;
		// env->NewLocalRef(obj);
		method = aMethod;
		// methodID = env->GetMethodID(cls, 
		append2sig("(");
	}

	sjniSCall(JNIEnv *aEnv, jclass aCls, jmethodID metID)
	{
		method = 0, sig = 0, len = 0, alen = 0, args = 0, nargs = 0, aargs = 0, obj = 0;

		env = aEnv;
		// cls = aCls;
		cls = (jclass) env->NewLocalRef(aCls); _SJNI_INC_REF_COUNT;
		// obj = aObj;
		// env->NewLocalRef(obj);
		methodID = metID;
	}

	void callV() { prepMethodID("V"); env->CallStaticVoidMethodA(cls, methodID, args); _SJNI_CHECK_JVM_EXC; }
	jboolean callZ() { prepMethodID("Z"); jboolean tmp = env->CallStaticBooleanMethodA(cls, methodID, args); _SJNI_CHECK_JVM_EXC; return tmp; }
	jbyte callB() { prepMethodID("B"); jbyte tmp = env->CallStaticByteMethodA(cls, methodID, args); _SJNI_CHECK_JVM_EXC; return tmp; }
	jchar callC() { prepMethodID("C"); jchar tmp = env->CallStaticCharMethodA(cls, methodID, args); _SJNI_CHECK_JVM_EXC; return tmp; }
	jshort callS() { prepMethodID("S"); jshort tmp = env->CallStaticShortMethodA(cls, methodID, args); _SJNI_CHECK_JVM_EXC; return tmp; }
	jint callI() { prepMethodID("I"); jint tmp = env->CallStaticIntMethodA(cls, methodID, args); _SJNI_CHECK_JVM_EXC; return tmp; }
	jlong callL() { prepMethodID("J"); jlong tmp = env->CallStaticLongMethodA(cls, methodID, args); _SJNI_CHECK_JVM_EXC; return tmp; }
	jfloat callF() { prepMethodID("F"); jfloat tmp = env->CallStaticFloatMethodA(cls, methodID, args); _SJNI_CHECK_JVM_EXC; return tmp; }
	jdouble callD() { prepMethodID("D"); jdouble tmp = env->CallStaticDoubleMethodA(cls, methodID, args); _SJNI_CHECK_JVM_EXC; return tmp; }
	jobject callO(const char *clz) { prepMethodID("L", clz); jobject o = env->CallStaticObjectMethodA(cls, methodID, args); _SJNI_CHECK_JVM_EXC; _SJNI_INC_REF_COUNT2(o); return o; }
	void callO(const char *clz, sjniObj &);
	void callA(sjniAry&);
	// sjniObj callO(const char *clz) { prepMethodID("L", clz); env->CallObjectMethod(obj, methodID, args); }

private:
	sjniSCall& operator=(const sjniSCall&) { return *this; }

protected:
	void prepMethodID(const char *type)
	{
		if (!methodID)
		{
			append2sig(")");
			append2sig(type);
			printf("GetStaticMethodID() cls = 0x%08X, method = %s, sig = %s\n", cls, method, sig);
			methodID = env->GetStaticMethodID(cls, method, sig);
			if (!methodID)
			{
				printf("Method %s %s not found\n", method, sig);
			}
			_SJNI_MNFE_Z(methodID, method, sig, cls);
		}

		printf("nargs = %d aargs = %d\n", nargs, aargs);
		for (int i = 0; i < nargs; i++)
		{
			printf("0x%08X\n", args[i].l);
		}
	}

	void prepMethodID(const char *type, const char *clz)
	{
		if (!methodID)
		{
			append2sig(")");
			append2sig(type);
			append2sig(clz);
			append2sig(";");
			methodID = env->GetStaticMethodID(cls, method, sig);
			if (!methodID)
			{
				printf("Method %s %s not found\n", method, sig);
			}
			_SJNI_MNFE_Z(methodID, method, sig, cls);
		}
	}
};

class sjniObj
{
public:
	sjniObj(): env(0), clsName(0), cls(0), obj(0) {}
	// sjniObj(JNIEnv *aEnv, 

	sjniObj(const sjniObj& other)
	{
		// *this = other;
		// clsName = strdup(clsName);
		env = other.env;
		clsName = strdup(other.clsName);
		if (other.cls) { cls = (jclass) env->NewLocalRef(other.cls); _SJNI_INC_REF_COUNT; }
		if (other.obj) { obj = env->NewLocalRef(other.obj); _SJNI_INC_REF_COUNT; }
	}

	sjniObj& operator=(const sjniObj& other)
	{
		if (clsName) free(clsName);
		
		if (cls) { env->DeleteLocalRef(cls); _SJNI_DEC_REF_COUNT; }
		if (obj) { env->DeleteLocalRef(obj); _SJNI_DEC_REF_COUNT; }

		env = other.env;
		clsName = strdup(other.clsName);

		if (other.cls) {
			cls = (jclass) env->NewLocalRef(other.cls);
			_SJNI_INC_REF_COUNT;
		} else
		{
			cls = 0;
		}
		
		if (other.obj)
		{
			obj = env->NewLocalRef(other.obj);
			_SJNI_INC_REF_COUNT;
		} else
		{
			obj = 0;
		}
		return *this;
	}

	sjniObj(JNIEnv *aEnv, /* const sjniObj &other, */ const char *aClsName, jobject aObj)
	{
		env = aEnv; // other.env;
		clsName = strdup(aClsName);
		cls = env->FindClass(aClsName); _SJNI_CNFE_Z(cls, aClsName); _SJNI_INC_REF_COUNT2(cls);
		obj = aObj; // env->NewLocalRef(
	}

	sjniObj(JNIEnv *aEnv, const char *aClsName, jclass aCls, jobject aObj)
	{
		env = aEnv;
		clsName = strdup(aClsName);
		// cls = aCls;
		cls = (jclass) env->NewLocalRef(aCls); _SJNI_INC_REF_COUNT;
		obj = aObj; // env->NewLocalRef(aObj);
	}
	// sjniObj(jobject, const sjniCls&);

	sjniObj(const sjniFld &f)
	{
		env = f.jenv();
		clsName = strdup(f.getClassName());
		cls = (jclass) env->NewLocalRef(f.jcls()); _SJNI_INC_REF_COUNT;
		obj = f.getO();
	}

	sjniObj(const sjniSFld &f)
	{
		env = f.jenv();
		clsName = strdup(f.getClassName());
		cls = (jclass) env->NewLocalRef(f.jcls()); _SJNI_INC_REF_COUNT;
		obj = f.getO();
	}

	~sjniObj()
	{
		if (obj) { env->DeleteLocalRef(obj); _SJNI_DEC_REF_COUNT; }
		if (cls) { env->DeleteLocalRef(cls); _SJNI_DEC_REF_COUNT; }
		free(clsName);
	}

	jobject newGlobalRef()
	{
		return env->NewGlobalRef(obj);
	}

	void deleteGlobalRef()
	{
		env->DeleteGlobalRef(obj);
	}

	jobject newLocalRef()
	{
		return env->NewLocalRef(obj);
	}

	void deleteLocalRef()
	{
		env->DeleteLocalRef(obj);
	}

	sjniCall operator<< (const char *met)
	{
		return sjniCall(env, cls, obj, met);
	}

	sjniCall call(const char *met)
	{
		return sjniCall(env, cls, obj, met);
	}

	sjniSCall sCall (const char *met)
	{
		return sjniSCall(env, cls, met);
	}

	// sjniCall operator<< (const sjniMet&);
	sjniCall operator<< (jmethodID metID)
	{
		return sjniCall(env, obj, metID);
	}

	sjniSFld sfld(const char *name, const char *sig)
	{
		return sjniSFld(env, cls, name, sig);
	}

	sjniFld fld(const char *name, const char *sig)
	{
		return sjniFld(env, cls, obj, name, sig);
	}

	sjniFld fld(const sjniFld&);
	
	sjniObj as(const char *otherClsName)
	{
		return sjniObj(env, otherClsName, cls, obj);
	}

	/* jboolean getZ(const char*);
	jbyte getB(const char*);
	jchar getC(const char*);
	jshort getS(const char*);
	jint getI(const char*);
	jlong getL(const char*);
	jfloat getF(const char*);
	jdouble getD(const char*);
	jobject getNO(const char*);
	sjniObj getO(const char*);

	void get(const char*, jboolean&);
	void get(const char*, jbyte&);
	void get(const char*, jchar&);
	void get(const char*, jshort&);
	void get(const char*, jint&);
	void get(const char*, jlong&);
	void get(const char*, jfloat&);
	void get(const char*, jdouble&);
	// void get(const char*, jboolean&);
	void get(const char*, jobject&);
	void get(const char*, sjniObj&);

	void set(const char*, jboolean);
	void set(const char*, jbyte);
	void set(const char*, jchar);
	void set(const char*, jshort);
	void set(const char*, jint);
	void set(const char*, jlong);
	void set(const char*, jfloat);
	void set(const char*, jdouble);
	void set(const char*, jobject);
	void set(const char*, const sjniObj&); */

	const char *getClsName() const { return clsName; }
	jobject jobj() const { return obj; }
	JNIEnv* jenv() const { return env; }
	bool isNull() { return (obj == 0); }
	bool notNull() { return (obj != 0); }

protected:
	JNIEnv *env;
	char *clsName;
	jclass cls;
	jobject obj;
};

class sjniCls
{
public:
	sjniCls(const sjniCls &other)
	{
		env = other.env;
		cls = (jclass) env->NewLocalRef(other.cls); _SJNI_INC_REF_COUNT;
		printf("other.cls = 0x%08X cls = 0x%08X\n", other.cls, cls);
		clsName = strdup(other.clsName);
	}

	sjniCls(JNIEnv *aEnv, const char *clz)
	{
		env = aEnv;
		printf("Trying to find class %s\n", clz);
		cls = env->FindClass(clz); _SJNI_CNFE_Z(cls, clz); _SJNI_INC_REF_COUNT;
		clsName = strdup(clz);
	}

	~sjniCls()
	{
		free(clsName);
		if (cls) { env->DeleteLocalRef(cls); _SJNI_DEC_REF_COUNT; }
	}

	void deleteLocalRef()
	{
		env->DeleteLocalRef(cls); _SJNI_DEC_REF_COUNT;
	}

	sjniMet met(const char *metName, const char *sig) { return sjniMet(env, cls, metName, sig); }
	sjniFld fld(const char *fldName, const char *sig) { return sjniFld(env, cls, fldName, sig); }
	sjniSFld sfld(const char *fldName, const char *sig) { return sjniSFld(env, cls, fldName, sig); }

	sjniObj nnew(const char *sig, ...)
	{
		jmethodID ctorID = env->GetMethodID(cls, "<init>", sig); _SJNI_MNFE_Z(ctorID, "<init>", sig, cls);
		va_list ap;
		va_start(ap, sig);
		jobject obj = env->NewObjectV(cls, ctorID, ap); _SJNI_INC_REF_COUNT2(obj);
		va_end(ap);
		return sjniObj (env, clsName, cls, obj);
	}

	sjniObj nnew(const char *sig, va_list ap)
	{
		jmethodID ctorID = env->GetMethodID(cls, "<init>", sig); _SJNI_MNFE_Z(ctorID, "<init>", sig, cls);
		jobject obj = env->NewObjectV(cls, ctorID, ap); _SJNI_INC_REF_COUNT2(obj);
		return sjniObj(env, clsName, cls, obj);
	}

	sjniSCall sCall(const char *met)
	{
		return sjniSCall(env, cls, met);
	}

	const char* name() { return clsName; }
	jclass jcls() { return cls; }

private:
	sjniCls& operator=(const sjniCls&) { return *this; }

private:
	JNIEnv *env;
	char *clsName;
	jclass cls;
};

class sjniStr: public sjniObj
{
public:
	sjniStr(JNIEnv *aEnv, const char* s)
	{
		env = aEnv;
		clsName = strdup("java/lang/String");
		cls = env->FindClass(clsName); _SJNI_CNFE_Z(cls, clsName); _SJNI_INC_REF_COUNT2(cls);
		obj = env->NewStringUTF(s); _SJNI_CHECK_JVM_EXC; _SJNI_INC_REF_COUNT2(obj);
		printf("String obj = 0x%08X (%s)\n", obj, s);
	}

	~sjniStr()
	{
		// env->DeleteLocalRef(cls);
		// env->DeleteLocalRef(obj);
	}
};

class sjniAry//: public sjniObj
{
public:
	sjniAry(): env(0), obj(0), arySig(0), curIdx(0)
	{
	}

	sjniAry(const sjniAry &aAry): env(0), obj(0), arySig(0), curIdx(0)
	{
		*this = aAry;
	}

	sjniAry& operator= (const sjniAry &aAry)
	{
		free(arySig);
		if (obj) { env->DeleteLocalRef(obj); _SJNI_DEC_REF_COUNT; }

		env = aAry.jenv();
		obj = env->NewLocalRef(aAry.jobj()); _SJNI_INC_REF_COUNT2(obj);
		arySig = strdup(aAry.sig());
		curIdx = 0;
		return *this;
	}

	sjniAry(const sjniObj &aObj): curIdx(0) // obj(aObj)
	{
		env = aObj.jenv();
		obj = env->NewLocalRef(aObj.jobj()); _SJNI_INC_REF_COUNT2(obj);
	}

	sjniAry(const char *sig): env(0), obj(0), curIdx(0)
	{
		arySig = (char*) malloc(strlen(sig) + 2);
		strcpy(arySig, "[");
		strcat(arySig, sig); // strdup(sig);
	}

	void receiveObject(JNIEnv *aEnv, jobject aObj)
	{
		env = aEnv;
		obj = aObj;
	}

	sjniAry(JNIEnv *aEnv, int len, const char *sig, const char *psig = 0, ...): curIdx(0)
	{
		va_list ap;
		va_start(ap, psig);
		create(aEnv, len, sig, psig, ap);
		va_end(ap);
	}

	sjniAry(JNIEnv *aEnv, int len, const char *sig, const char *psig, va_list ap): curIdx(0)
	{
		create(aEnv, len, sig, psig, ap);
	}

private:
	void create(JNIEnv *aEnv, int len, const char *sig, const char *psig, va_list ap)
	{
		env = aEnv;
		// jobject t;
		switch(*sig)
		{
		case 'Z':
			obj = env->NewBooleanArray(len);
			break;
		case 'B':
			obj = env->NewByteArray(len);
			break;
		case 'C':
			obj = env->NewCharArray(len);
			break;
		case 'S':
			obj = env->NewShortArray(len);
			break;
		case 'I':
			obj = env->NewIntArray(len);
			break;
		case 'J':
			obj = env->NewLongArray(len);
			break;
		case 'F':
			obj = env->NewFloatArray(len);
			break;
		case 'D':
			obj = env->NewDoubleArray(len);
			break;
		case 'L':
			{
				char *buf = strdup(sig + 1);
				buf[strlen(buf)-1] = 0;
				jclass cls = env->FindClass(buf);
				jmethodID ctorID = env->GetMethodID(cls, "<init>", psig); _SJNI_MNFE_Z(ctorID, "<init>", sig, cls);
				// va_list ap;
				// va_start (ap, psig);
				jobject init = env->NewObjectV(cls, ctorID, ap); _SJNI_CHECK_JVM_EXC;
				// va_end(ap);
				obj = env->NewObjectArray(len, cls, init); _SJNI_CHECK_JVM_EXC;

				char *buf2 = (char*) calloc(strlen(sig) + 2, 1);
				strcat(buf2, "[");
				strcat(buf2, sig);
				arySig = buf2;

				// obj = sjniObj(aEnv, buf2, cls, t);

				env->DeleteLocalRef(init);
				env->DeleteLocalRef(cls);
				free(buf);
				// free(buf2);
			}
			_SJNI_INC_REF_COUNT2(obj);
			return;
		}
		{
			_SJNI_CHECK_JVM_EXC;
			_SJNI_INC_REF_COUNT2(obj);
			// char buf[3];
			arySig = (char*) malloc(3);
			arySig[0] = '[';
			arySig[1] = *sig;
			arySig[2] = 0;
			// jclass cls = env->FindClass(buf);
			// obj = sjniObj(aEnv, buf, cls, t);
		}
	}

public:
	~sjniAry()
	{
		free(arySig);
		if (obj) { env->DeleteLocalRef(obj); _SJNI_DEC_REF_COUNT; }
	}

	jboolean getZ(int idx) { jboolean buf; env->GetBooleanArrayRegion((jbooleanArray) obj, idx, 1, &buf); _SJNI_CHECK_JVM_EXC; return buf; }
	jbyte getB(int idx) { jbyte buf; env->GetByteArrayRegion((jbyteArray) obj, idx, 1, &buf); _SJNI_CHECK_JVM_EXC; return buf; }
	jchar getC(int idx) { jchar buf; env->GetCharArrayRegion((jcharArray) obj, idx, 1, &buf); _SJNI_CHECK_JVM_EXC; return buf; }
	jshort getS(int idx) { jshort buf; env->GetShortArrayRegion((jshortArray) obj, idx, 1, &buf); _SJNI_CHECK_JVM_EXC; return buf; }
	jint getI(int idx) { jint buf; env->GetIntArrayRegion((jintArray) obj, idx, 1, &buf); _SJNI_CHECK_JVM_EXC; return buf; }
	jlong getL(int idx) { jlong buf; env->GetLongArrayRegion((jlongArray) obj, idx, 1, &buf); _SJNI_CHECK_JVM_EXC; return buf; }
	jfloat getF(int idx) { jfloat buf; env->GetFloatArrayRegion((jfloatArray) obj, idx, 1, &buf); _SJNI_CHECK_JVM_EXC; return buf; }
	jdouble getD(int idx) { jdouble buf; env->GetDoubleArrayRegion((jdoubleArray) obj, idx, 1, &buf); _SJNI_CHECK_JVM_EXC; return buf; }
	jobject getO(int idx) { jobject o = env->GetObjectArrayElement((jobjectArray) obj, idx); _SJNI_INC_REF_COUNT2(o); _SJNI_CHECK_JVM_EXC; return o; } // , 1, &buf); return buf; }
	sjniObj getSO(int idx)
	{
		char *buf = strdup(arySig), *bufp = buf;
		while (*bufp == '[') bufp++;
		bufp[strlen(bufp)-1] = 0;
		jclass cls = env->FindClass(bufp); _SJNI_CNFE_Z(cls, bufp); _SJNI_INC_REF_COUNT2(cls); // arySig + 1);
		sjniObj o(env, arySig + 1, cls, getO(idx));
		env->DeleteLocalRef(cls); _SJNI_DEC_REF_COUNT;
		free(buf);
		return o;
	}

	void set(int idx, jboolean z) { env->SetBooleanArrayRegion((jbooleanArray) obj, idx, 1, &z); _SJNI_CHECK_JVM_EXC; }
	void set(int idx, jbyte b) { env->SetByteArrayRegion((jbyteArray) obj, idx, 1, &b); _SJNI_CHECK_JVM_EXC; }
	void set(int idx, jchar c) { env->SetCharArrayRegion((jcharArray) obj, idx, 1, &c); _SJNI_CHECK_JVM_EXC; }
	void set(int idx, jshort s) { env->SetShortArrayRegion((jshortArray) obj, idx, 1, &s); _SJNI_CHECK_JVM_EXC; }
	void set(int idx, jint i) { env->SetIntArrayRegion((jintArray) obj, idx, 1, &i); _SJNI_CHECK_JVM_EXC; }
	void set(int idx, jlong l) { env->SetLongArrayRegion((jlongArray) obj, idx, 1, &l); _SJNI_CHECK_JVM_EXC; }
	void set(int idx, jfloat f) { env->SetFloatArrayRegion((jfloatArray) obj, idx, 1, &f); _SJNI_CHECK_JVM_EXC; }
	void set(int idx, jdouble d) { env->SetDoubleArrayRegion((jdoubleArray) obj, idx, 1, &d); _SJNI_CHECK_JVM_EXC; }
	void set(int idx, jobject o) { env->SetObjectArrayElement((jobjectArray) obj, idx, o); _SJNI_CHECK_JVM_EXC; }

	sjniAry& operator<< (jboolean z) { set(curIdx, z); curIdx++; return *this; }
	sjniAry& operator<< (jbyte b) { set(curIdx, b); curIdx++; return *this; }
	sjniAry& operator<< (jchar c) { set(curIdx, c); curIdx++; return *this; }
	sjniAry& operator<< (jshort s) { set(curIdx, s); curIdx++; return *this; }
	sjniAry& operator<< (jint i) { set(curIdx, i); curIdx++; return *this; }
	sjniAry& operator<< (jlong l) { set(curIdx, l); curIdx++; return *this; }
	sjniAry& operator<< (jfloat f) { set(curIdx, f); curIdx++; return *this; }
	sjniAry& operator<< (jdouble d) { set(curIdx, d); curIdx++; return *this; }
	sjniAry& operator<< (jobject o) { set(curIdx, o); curIdx++; return *this; }
	sjniAry& operator<< (const sjniObj &o) { set(curIdx, o.jobj()); return *this; }

	void setCurIdx(int idx) { curIdx = idx; }

	jsize len() { jsize tmp = env->GetArrayLength((jarray) obj); _SJNI_CHECK_JVM_EXC; return tmp; }

	JNIEnv* jenv() const { return env; }
	jobject jobj() const { return obj; }
	const char* sig() const { return arySig; }

private:
	JNIEnv *env;
	jobject obj;
	char *arySig;
	jint curIdx;
};

class sjniPkg
{
public:
	sjniPkg(JNIEnv *aEnv, const char *aPkgPrefix)
	{
		// vm = other.vm;
		// env = other.env;
		env = aEnv; // .jenv();
		pkgPrefix = strdup(aPkgPrefix);
	}

	~sjniPkg()
	{
		free(pkgPrefix);
	}

	sjniCls cls(const char *name)
	{
		char *buf;
		buf = (char*) calloc(strlen(pkgPrefix) + strlen(name) + 2, 1);
		strcat(buf, pkgPrefix);
		strcat(buf, "/");
		strcat(buf, name);
		printf("Loading class %s\n", buf);
		sjniCls tmp = sjniCls(env, buf); // name);
		free(buf);
		return tmp;
	}

	sjniObj nnew(const char *clz, const char *sig, ...)
	{
		sjniCls c = cls(clz);
		va_list ap;
		va_start(ap, sig);
		sjniObj o = c.nnew(sig, ap);
		va_end(ap);
		return o;
	}

private:
	sjniPkg& operator=(const sjniPkg&) { return *this; }

private:
	JNIEnv *env;
	char *pkgPrefix;
};

class sjniEnv
{
public:
	sjniEnv(): vm(0), env(0) { create(); }
	sjniEnv(jint version): vm(0), env(0) { create(version); }
	sjniEnv(JavaVM *aVm, jint version): vm(0), env(0) { create(aVm, version); }
	sjniEnv(JNIEnv *aEnv): vm(0), env(0) { create(aEnv); }

	/* ~sjniEnv()
	{
		destroy();
	} */

	void destroy()
	{
		if (vm)
		{
			vm->DestroyJavaVM();
			vm = 0;
		}
		env = 0;
	}

	void create() { create(JNI_VERSION_1_6); }
	void create(jint version)
	{
		JavaVMInitArgs args;
		JavaVMOption options[1];
		args.version = version;
		args.nOptions = 0;
		args.options = &options[0]; 
		args.ignoreUnrecognized = JNI_TRUE;
		char *cp = getenv("CLASSPATH");
		char *buf(0);
		if (cp)
		{
			buf = (char*) malloc(strlen(cp) + 32);
			strcpy(buf, "-Djava.class.path=");
			strcat(buf, cp);
			options[0].optionString = buf;
			args.nOptions++;
			printf("%s\n", buf);
		}
		// JNI_GetDefaultJavaVMInitArgs(&args);
		JNI_CreateJavaVM(&vm, (void**) &env, &args);
		free(buf);
	}
	void create(JavaVM *aVm, jint version)
	{
		vm = aVm;
		aVm->GetEnv((void**) &env, version);
	}
	void create(JNIEnv *aEnv)
	{
		env = aEnv;
	}

	sjniPkg pkg(const char *prefix)
	{
		return sjniPkg(env, prefix);
	}

	sjniCls cls(const char *name)
	{
		// env->FindClass("name");
		return sjniCls(env, name);
	}

	sjniObj str(const char *s)
	{
		return sjniStr(env, s);
	}

	sjniObj obj(const char *clsName, jobject obj)
	{
		return sjniObj(env, clsName, obj);
	}

	sjniAry ary(int len, const char *sig, const char *psig = 0, ...)
	{
		va_list ap;
		va_start(ap, psig);
		sjniAry a = sjniAry(env, len, sig, psig, ap);
		va_end(ap);
		return a;
	}

	sjniAry ary(const char *sig, jobject aObj)
	{
		sjniAry a(sig);
		a.receiveObject(env, aObj);
		return a;
	}

	sjniObj nnew(const char *clz, const char *sig, ...)
	{
		sjniCls c = cls(clz);
		va_list ap;
		va_start(ap, sig);
		sjniObj o = c.nnew(sig, ap);
		va_end(ap);
		return o;
	}

	jboolean hasException()
	{
		return env->ExceptionCheck();
	}

	bool clearException()
	{
		jthrowable exc = env->ExceptionOccurred(); _SJNI_INC_REF_COUNT2(exc);
		if (exc)
		{
			env->ExceptionClear(); // exc);
			env->DeleteLocalRef(exc); _SJNI_DEC_REF_COUNT;
			return true;
		}
		return false;
	}

	jthrowable exception()
	{
		return env->ExceptionOccurred();
	}

	bool describeAndClearException()
	{
		jthrowable exc = env->ExceptionOccurred(); _SJNI_INC_REF_COUNT2(exc);
		if (exc)
		{
			env->ExceptionDescribe();
			env->ExceptionClear();
			env->DeleteLocalRef(exc); _SJNI_DEC_REF_COUNT;
			return true;
		}
		return false;
	}

	sjniObj getAndDescribeAndClearException()
	{
		jthrowable exc = env->ExceptionOccurred(); _SJNI_INC_REF_COUNT2(exc);
		if (exc)
		{
			env->ExceptionDescribe();
			env->ExceptionClear();
			// env->DeleteLocalRef(exc);
			return obj("Ljava/lang/Throwable;", exc);
		}
		return sjniObj();
	}

	static void jfree(void*);

	JavaVM *jvm() const { return vm; }
	JNIEnv* jenv() const { return env; }

private:
	sjniEnv& operator=(const sjniEnv&) { return *this; }

private:
	JavaVM *vm;
	JNIEnv *env;
};

#if defined(_SJNI_EXCEPTIONS_) && defined(_SJNI_JAVA_EXCEPTIONS_)
void sjniExceptionHelper::throwIfJavaExceptionOccured(JNIEnv *aEnv)
{
	jthrowable exc = aEnv->ExceptionOccurred(); _SJNI_INC_REF_COUNT2(exc);
	if (exc)
	{
#if defined(_SJNI_DESCRIBE_JAVA_EXCEPTIONS_)
		aEnv->ExceptionDescribe();
#endif // _SJNI_DESCRIBE_JAVA_EXCEPTIONS_
		aEnv->ExceptionClear();
		printf("exc = 0x%08X\n", exc);
		sjniObj excObj(aEnv, "Ljava/lang/Throwable;", exc);
		sjniObj msgObj;
		(excObj << "getMessage").callO("java/lang/String", msgObj);
		jboolean isCopy;
		const char *msgChars = aEnv->GetStringUTFChars((jstring) msgObj.jobj(), &isCopy);
		sjniException e(msgChars);
		if (isCopy) sjniEnv::jfree((void*) msgChars);
		throw e;
	}
}
#endif // defined(_SJNI_EXCEPTIONS) && defined(_SJNI_JAVA_EXCEPTIONS_)

#endif // _SJNI_H_
