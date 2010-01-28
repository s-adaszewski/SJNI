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
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
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

class sjniMet;
class sjniFld;
class sjniCls;
class sjniCall;
class sjniObj;
class sjniPkg;
class sjniAry;

class sjniMet
{
public:
	sjniMet(JNIEnv *env, jclass cls, const char *name, const char *sig)
	{
		methodID = env->GetMethodID(cls, name, sig);
	}

	jmethodID jmet() { return methodID; }

private:
	jmethodID methodID;
};

class sjniFld
{
public:
	sjniFld(JNIEnv *aEnv, /* const char *aClsName, jclass aCls, */ const char *name, const char *sig);
	
	sjniFld(JNIEnv *aEnv, /* const char *aClassName,  jclass aCls, */ jobject aObj, const char *name, const char *sig);
	
	~sjniFld()
	{
		free(clsName);
	}

	sjniFld& operator>> (jboolean &z) { z = env->GetBooleanField(obj, fieldID); return *this; }
	sjniFld& operator>> (jbyte &b) { b = env->GetByteField(obj, fieldID); return *this; }
	sjniFld& operator>> (jchar &c) { c = env->GetCharField(obj, fieldID); return *this; }
	sjniFld& operator>> (jshort &s) { s = env->GetShortField(obj, fieldID); return *this; }
	sjniFld& operator>> (jint &i) { i = env->GetIntField(obj, fieldID); return *this; }
	sjniFld& operator>> (jlong &l) { l = env->GetLongField(obj, fieldID); return *this; }
	sjniFld& operator>> (jfloat &f) { f = env->GetFloatField(obj, fieldID); return *this; }
	sjniFld& operator>> (jdouble &d) { d = env->GetDoubleField(obj, fieldID); return *this; }
	sjniFld& operator>> (jobject &o) { o = env->GetObjectField(obj, fieldID); return *this; }
	// sjniFld& operator>> (sjniObj &o) { o = sjniObj(env->GetObjectField(obj, fieldID)); return *this; }

	sjniFld& operator<< (jboolean z) { env->SetBooleanField(obj, fieldID, z); return *this; }
	sjniFld& operator<< (jbyte b) { env->SetByteField(obj, fieldID, b); return *this; }
	sjniFld& operator<< (jchar c) { env->SetCharField(obj, fieldID, c); return *this; }
	sjniFld& operator<< (jshort s) { env->SetShortField(obj, fieldID, s); return *this; }
	sjniFld& operator<< (jint i) { env->SetIntField(obj, fieldID, i); return *this; }
	sjniFld& operator<< (jlong l) { env->SetLongField(obj, fieldID, l); return *this; }
	sjniFld& operator<< (jfloat f) { env->SetFloatField(obj, fieldID, f); return *this; }
	sjniFld& operator<< (jdouble d) { env->SetDoubleField(obj, fieldID, d); return *this; }
	sjniFld& operator<< (jobject o) { env->SetObjectField(obj, fieldID, o); return *this; }
	// sjniFld& operator<< (const sjniObj &o) { env->SetObjectField(obj, fieldID, o.jobj()); return *this; }

	jboolean getZ() { return env->GetBooleanField(obj, fieldID); }
	jbyte getB() { return env->GetByteField(obj, fieldID); }
	jchar getC() { return env->GetCharField(obj, fieldID); }
	jshort getS() { return env->GetShortField(obj, fieldID); }
	jint getI() { return env->GetIntField(obj, fieldID); }
	jlong getL() { return env->GetLongField(obj, fieldID); }
	jfloat getF() { return env->GetFloatField(obj, fieldID); }
	jdouble getD() { return env->GetDoubleField(obj, fieldID); }
	jobject getO() const { return env->GetObjectField(obj, fieldID); }
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

	JNIEnv* jenv() const { return env; }
	const char* getClassName() const { return clsName; }
	jclass jcls() const { return cls; }

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
	}

	/* sjniSFld(JNIEnv *aEnv, jclass cls, jobject aObj, const char *name, const char *sig)
	{
		env = aEnv;
		fieldID = env->GetFieldID(cls, name, sig);
		obj = aObj;
	} */

	sjniSFld& operator>> (jboolean &z) { z = env->GetStaticBooleanField(cls, fieldID); return *this; }
	sjniSFld& operator>> (jbyte &b) { b = env->GetStaticByteField(cls, fieldID); return *this; }
	sjniSFld& operator>> (jchar &c) { c = env->GetStaticCharField(cls, fieldID); return *this; }
	sjniSFld& operator>> (jshort &s) { s = env->GetStaticShortField(cls, fieldID); return *this; }
	sjniSFld& operator>> (jint &i) { i = env->GetStaticIntField(cls, fieldID); return *this; }
	sjniSFld& operator>> (jlong &l) { l = env->GetStaticLongField(cls, fieldID); return *this; }
	sjniSFld& operator>> (jfloat &f) { f = env->GetStaticFloatField(cls, fieldID); return *this; }
	sjniSFld& operator>> (jdouble &d) { d = env->GetStaticDoubleField(cls, fieldID); return *this; }
	sjniSFld& operator>> (jobject &o) { o = env->GetStaticObjectField(cls, fieldID); return *this; }
	// sjniFld& operator>> (sjniObj &o) { o = sjniObj(env->GetObjectField(obj, fieldID)); return *this; }

	sjniSFld& operator<< (jboolean z) { env->SetStaticBooleanField(cls, fieldID, z); return *this; }
	sjniSFld& operator<< (jbyte b) { env->SetStaticByteField(cls, fieldID, b); return *this; }
	sjniSFld& operator<< (jchar c) { env->SetStaticCharField(cls, fieldID, c); return *this; }
	sjniSFld& operator<< (jshort s) { env->SetStaticShortField(cls, fieldID, s); return *this; }
	sjniSFld& operator<< (jint i) { env->SetStaticIntField(cls, fieldID, i); return *this; }
	sjniSFld& operator<< (jlong l) { env->SetStaticLongField(cls, fieldID, l); return *this; }
	sjniSFld& operator<< (jfloat f) { env->SetStaticFloatField(cls, fieldID, f); return *this; }
	sjniSFld& operator<< (jdouble d) { env->SetStaticDoubleField(cls, fieldID, d); return *this; }
	sjniSFld& operator<< (jobject o) { env->SetStaticObjectField(cls, fieldID, o); return *this; }
	// sjniFld& operator<< (const sjniObj &o) { env->SetObjectField(obj, fieldID, o.jobj()); return *this; }

	jboolean getZ() { return env->GetStaticBooleanField(cls, fieldID); }
	jbyte getB() { return env->GetStaticByteField(cls, fieldID); }
	jchar getC() { return env->GetStaticCharField(cls, fieldID); }
	jshort getS() { return env->GetStaticShortField(cls, fieldID); }
	jint getI() { return env->GetStaticIntField(cls, fieldID); }
	jlong getL() { return env->GetStaticLongField(cls, fieldID); }
	jfloat getF() { return env->GetStaticFloatField(cls, fieldID); }
	jdouble getD() { return env->GetStaticDoubleField(cls, fieldID); }
	jobject getO() const { return env->GetStaticObjectField(cls, fieldID); }
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

	JNIEnv* jenv() const { return env; }
	const char* getClassName() const { return clsName; }
	jclass jcls() const { return fieldCls; }

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
		cls = (jclass) env->NewLocalRef(aCls);
		// obj = aObj;
		obj = env->NewLocalRef(aObj);
		method = aMethod;
		// methodID = env->GetMethodID(cls, 
		append2sig("(");
	}

	sjniCall(JNIEnv *aEnv, jobject aObj, jmethodID metID): method(0), sig(0), len(0), alen(0), args(0), nargs(0), aargs(0), cls(0)
	{
		env = aEnv;
		// obj = aObj;
		obj = env->NewLocalRef(aObj);
		methodID = metID;
	}

	~sjniCall()
	{
		if (cls) env->DeleteLocalRef(cls);
		if (obj) env->DeleteLocalRef(obj);
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

	void callV() { prepMethodID("V"); env->CallVoidMethodA(obj, methodID, args); }
	jboolean callZ() { prepMethodID("Z"); env->CallBooleanMethodA(obj, methodID, args); }
	jbyte callB() { prepMethodID("B"); env->CallByteMethodA(obj, methodID, args); }
	jchar callC() { prepMethodID("C"); env->CallCharMethodA(obj, methodID, args); }
	jshort callS() { prepMethodID("S"); env->CallShortMethodA(obj, methodID, args); }
	jint callI() { prepMethodID("I"); env->CallIntMethodA(obj, methodID, args); }
	jlong callL() { prepMethodID("J"); env->CallLongMethodA(obj, methodID, args); }
	jfloat callF() { prepMethodID("F"); env->CallFloatMethodA(obj, methodID, args); }
	jdouble callD() { prepMethodID("D"); env->CallDoubleMethodA(obj, methodID, args); }
	jobject callO(const char *clz) { prepMethodID("L", clz); env->CallObjectMethodA(obj, methodID, args); }
	// sjniObj callO(const char *clz) { prepMethodID("L", clz); env->CallObjectMethod(obj, methodID, args); }

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

	void prepMethodID(const char *type)
	{
		if (!methodID)
		{
			append2sig(")");
			append2sig(type);
			printf("GetMethodID() cls = 0x%08X, method = %s, sig = %s\n", cls, method, sig);
			methodID = env->GetMethodID(cls, method, sig);
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
			methodID = env->GetMethodID(cls, method, sig);
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
		cls = (jclass) env->NewLocalRef(aCls);
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
		cls = (jclass) env->NewLocalRef(aCls);
		// obj = aObj;
		// env->NewLocalRef(obj);
		methodID = metID;
	}

	void callV() { prepMethodID("V"); env->CallStaticVoidMethodA(cls, methodID, args); }
	jboolean callZ() { prepMethodID("Z"); env->CallStaticBooleanMethodA(cls, methodID, args); }
	jbyte callB() { prepMethodID("B"); env->CallStaticByteMethodA(cls, methodID, args); }
	jchar callC() { prepMethodID("C"); env->CallStaticCharMethodA(cls, methodID, args); }
	jshort callS() { prepMethodID("S"); env->CallStaticShortMethodA(cls, methodID, args); }
	jint callI() { prepMethodID("I"); env->CallStaticIntMethodA(cls, methodID, args); }
	jlong callL() { prepMethodID("J"); env->CallStaticLongMethodA(cls, methodID, args); }
	jfloat callF() { prepMethodID("F"); env->CallStaticFloatMethodA(cls, methodID, args); }
	jdouble callD() { prepMethodID("D"); env->CallStaticDoubleMethodA(cls, methodID, args); }
	jobject callO(const char *clz) { prepMethodID("L", clz); env->CallStaticObjectMethodA(cls, methodID, args); }
	// sjniObj callO(const char *clz) { prepMethodID("L", clz); env->CallObjectMethod(obj, methodID, args); }

protected:
	void prepMethodID(const char *type)
	{
		if (!methodID)
		{
			append2sig(")");
			append2sig(type);
			printf("GetStaticMethodID() cls = 0x%08X, method = %s, sig = %s\n", cls, method, sig);
			methodID = env->GetStaticMethodID(cls, method, sig);
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
		cls = (jclass) env->NewLocalRef(other.cls);
		obj = env->NewLocalRef(other.obj);
	}

	sjniObj(JNIEnv *aEnv, const char *aClsName, jclass aCls, jobject aObj)
	{
		env = aEnv;
		clsName = strdup(aClsName);
		// cls = aCls;
		cls = (jclass) env->NewLocalRef(aCls);
		obj = aObj; // env->NewLocalRef(aObj);
	}
	// sjniObj(jobject, const sjniCls&);

	sjniObj(const sjniFld &f)
	{
		env = f.jenv();
		clsName = strdup(f.getClassName());
		cls = (jclass) env->NewLocalRef(f.jcls());
		obj = f.getO();
	}

	sjniObj(const sjniSFld &f)
	{
		env = f.jenv();
		clsName = strdup(f.getClassName());
		cls = (jclass) env->NewLocalRef(f.jcls());
		obj = f.getO();
	}

	~sjniObj()
	{
		env->DeleteLocalRef(obj);
		env->DeleteLocalRef(cls);
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

	sjniCall sCall (const char *met)
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
		return sjniFld(env, obj, name, sig);
	}

	sjniFld fld(const sjniFld&);

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
		cls = (jclass) env->NewLocalRef(other.cls);
		printf("other.cls = 0x%08X cls = 0x%08X\n", other.cls, cls);
		clsName = strdup(other.clsName);
	}

	sjniCls(JNIEnv *aEnv, const char *clz)
	{
		env = aEnv;
		printf("Trying to find class %s\n", clz);
		cls = env->FindClass(clz);
		clsName = strdup(clz);
	}

	~sjniCls()
	{
		free(clsName);
		env->DeleteLocalRef(cls);
	}

	void deleteLocalRef()
	{
		env->DeleteLocalRef(cls);
	}

	sjniMet met(const char *metName, const char *sig) { return sjniMet(env, cls, metName, sig); }
	sjniFld fld(const char *fldName, const char *sig) { return sjniFld(env, cls, fldName, sig); }
	sjniSFld sfld(const char *fldName, const char *sig) { return sjniSFld(env, cls, fldName, sig); }

	sjniObj nnew(const char *sig, ...)
	{
		jmethodID ctorID = env->GetMethodID(cls, "<init>", sig);
		va_list ap;
		va_start(ap, sig);
		jobject obj = env->NewObjectV(cls, ctorID, ap);
		va_end(ap);
		return sjniObj (env, clsName, cls, obj);
	}

	sjniObj nnew(const char *sig, va_list ap)
	{
		jmethodID ctorID = env->GetMethodID(cls, "<init>", sig);
		jobject obj = env->NewObjectV(cls, ctorID, ap);
		return sjniObj(env, clsName, cls, obj);
	}

	sjniSCall sCall(const char *met)
	{
		return sjniSCall(env, cls, met);
	}

	const char* name() { return clsName; }
	jclass jcls() { return cls; }

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
		cls = env->FindClass(clsName);
		obj = env->NewStringUTF(s);
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
	sjniAry(const sjniObj &aObj): curIdx(0) // obj(aObj)
	{
		env = aObj.jenv();
		obj = env->NewLocalRef(aObj.jobj());
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
				jmethodID ctorID = env->GetMethodID(cls, "<init>", psig);
				// va_list ap;
				// va_start (ap, psig);
				jobject init = env->NewObjectV(cls, ctorID, ap);
				// va_end(ap);
				obj = env->NewObjectArray(len, cls, init);

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
			return;
		}
		{
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
		env->DeleteLocalRef(obj);
	}

	jboolean getZ(int idx) { jboolean buf; env->GetBooleanArrayRegion((jbooleanArray) obj, idx, 1, &buf); return buf; }
	jbyte getB(int idx) { jbyte buf; env->GetByteArrayRegion((jbyteArray) obj, idx, 1, &buf); return buf; }
	jchar getC(int idx) { jchar buf; env->GetCharArrayRegion((jcharArray) obj, idx, 1, &buf); return buf; }
	jshort getS(int idx) { jshort buf; env->GetShortArrayRegion((jshortArray) obj, idx, 1, &buf); return buf; }
	jint getI(int idx) { jint buf; env->GetIntArrayRegion((jintArray) obj, idx, 1, &buf); return buf; }
	jlong getL(int idx) { jlong buf; env->GetLongArrayRegion((jlongArray) obj, idx, 1, &buf); return buf; }
	jfloat getF(int idx) { jfloat buf; env->GetFloatArrayRegion((jfloatArray) obj, idx, 1, &buf); return buf; }
	jdouble getD(int idx) { jdouble buf; env->GetDoubleArrayRegion((jdoubleArray) obj, idx, 1, &buf); return buf; }
	jobject getO(int idx) { return env->GetObjectArrayElement((jobjectArray) obj, idx); } // , 1, &buf); return buf; }

	void set(int idx, jboolean z) { env->SetBooleanArrayRegion((jbooleanArray) obj, idx, 1, &z); }
	void set(int idx, jbyte b) { env->SetByteArrayRegion((jbyteArray) obj, idx, 1, &b); }
	void set(int idx, jchar c) { env->SetCharArrayRegion((jcharArray) obj, idx, 1, &c); }
	void set(int idx, jshort s) { env->SetShortArrayRegion((jshortArray) obj, idx, 1, &s); }
	void set(int idx, jint i) { env->SetIntArrayRegion((jintArray) obj, idx, 1, &i); }
	void set(int idx, jlong l) { env->SetLongArrayRegion((jlongArray) obj, idx, 1, &l); }
	void set(int idx, jfloat f) { env->SetFloatArrayRegion((jfloatArray) obj, idx, 1, &f); }
	void set(int idx, jdouble d) { env->SetDoubleArrayRegion((jdoubleArray) obj, idx, 1, &d); }
	void set(int idx, jobject o) { env->SetObjectArrayElement((jobjectArray) obj, idx, o); }

	sjniAry& operator<< (jboolean z) { set(curIdx, z); curIdx++; return *this; }
	sjniAry& operator<< (jbyte b) { set(curIdx, b); curIdx++; return *this; }
	sjniAry& operator<< (jchar c) { set(curIdx, c); curIdx++; return *this; }
	sjniAry& operator<< (jshort s) { set(curIdx, s); curIdx++; return *this; }
	sjniAry& operator<< (jint i) { set(curIdx, i); curIdx++; return *this; }
	sjniAry& operator<< (jlong l) { set(curIdx, l); curIdx++; return *this; }
	sjniAry& operator<< (jfloat f) { set(curIdx, f); curIdx++; return *this; }
	sjniAry& operator<< (jdouble d) { set(curIdx, d); curIdx++; return *this; }
	sjniAry& operator<< (jobject o) { set(curIdx, o); curIdx++; return *this; }

	void setCurIdx(int idx) { curIdx = idx; }

	jsize len() { return env->GetArrayLength((jarray) obj); }

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
		args.version = version;
		args.nOptions = 0;
		args.options = 0; 
		args.ignoreUnrecognized = JNI_TRUE;
		// JNI_GetDefaultJavaVMInitArgs(&args);
		JNI_CreateJavaVM(&vm, (void**) &env, &args);
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

	sjniAry ary(int len, const char *sig, const char *psig = 0, ...)
	{
		va_list ap;
		va_start(ap, psig);
		sjniAry a = sjniAry(env, len, sig, psig, ap);
		va_end(ap);
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

	JavaVM *jvm() const { return vm; }
	JNIEnv* jenv() const { return env; }

private:
	JavaVM *vm;
	JNIEnv *env;
};

#endif // _SJNI_H_
