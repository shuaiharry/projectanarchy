/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */


#define CHECK_SUCCESS( methodName )                       \
  if ( methodName == NULL )                             \
    {                                                     \
    printf( "++ unable to find '" #methodName "'\n" );  \
    env->ExceptionClear();                              \
    jvm->DetachCurrentThread();                         \
    return;                                             \
    }

class VNativeMobileDialogAndroid : public IVNativeMobileDialog
{
public:
  VNativeMobileDialogAndroid()
  {
    m_eState = WAITING;
    m_iNumButtons = 0;
    m_dialog = NULL;
  }

  virtual ~VNativeMobileDialogAndroid()
  {
    Close();
  }

  virtual void SetTitle(const char* pTitle) HKV_OVERRIDE
  {
    m_titleText = pTitle;
  }

  virtual void SetText(const char* pText) HKV_OVERRIDE
  {
    m_text = pText;

    if(m_dialog)
    {
      ANativeActivity* activity = AndroidApplication->activity; 

      JNIEnv *env = 0;
      JavaVM *jvm = activity->vm;
      jvm->AttachCurrentThread(&env, NULL);

      jmethodID method_VNativeMobileDialogAndroid_ChangeText = env->GetStaticMethodID(m_clazz_Dialog, "ChangeText",
        "(Landroid/app/Activity;Lcom/havok/Vision/VNativeMobileDialogAndroid;Ljava/lang/String;)V");
      CHECK_SUCCESS(method_VNativeMobileDialogAndroid_ChangeText);

      env->CallStaticVoidMethod(m_clazz_Dialog, method_VNativeMobileDialogAndroid_ChangeText, activity->clazz, m_dialog, env->NewStringUTF(pText));

      jvm->DetachCurrentThread();
    }
  }

  virtual int AddButton(const char* pButtonText) HKV_OVERRIDE
  {
    VASSERT_MSG(m_iNumButtons < 3, "Can only add at most 3 buttons on Android");
    m_buttonText[m_iNumButtons] = pButtonText;
    return m_iNumButtons++;
  }

  virtual void Show() HKV_OVERRIDE
  {
    ANativeActivity* activity = AndroidApplication->activity; 

    JNIEnv *env = 0;
    JavaVM *jvm = activity->vm;
    jvm->AttachCurrentThread(&env, NULL);

    jclass class_activity = env->GetObjectClass(activity->clazz);

    jmethodID method_getClassLoader = env->GetMethodID(class_activity, "getClassLoader", "()Ljava/lang/ClassLoader;");
    CHECK_SUCCESS( method_getClassLoader );

    jobject obj_classLoader = env->CallObjectMethod(activity->clazz, method_getClassLoader);
    CHECK_SUCCESS( obj_classLoader );

    jclass class_classLoader = env->FindClass("java/lang/ClassLoader");
    CHECK_SUCCESS( class_classLoader );

    jmethodID method_classLoader_loadClass = env->GetMethodID(class_classLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    CHECK_SUCCESS( method_classLoader_loadClass );

    jstring obj_strClassName = env->NewStringUTF("com.havok.Vision.VNativeMobileDialogAndroid");
    CHECK_SUCCESS( obj_strClassName );

    m_clazz_Dialog = (jclass)env->CallObjectMethod(obj_classLoader, method_classLoader_loadClass, obj_strClassName);
    CHECK_SUCCESS( m_clazz_Dialog );

    if (env->ExceptionOccurred())
    {
      env->ExceptionDescribe();
      env->ExceptionClear();
      jvm->DetachCurrentThread();
      return;
    }

    jmethodID method_VNativeMobileDialogAndroid_CreateAndShowInstance = env->GetStaticMethodID(m_clazz_Dialog, "CreateAndShowInstance",
      "(JLandroid/app/Activity;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    CHECK_SUCCESS(method_VNativeMobileDialogAndroid_CreateAndShowInstance);

    env->CallStaticVoidMethod(m_clazz_Dialog, method_VNativeMobileDialogAndroid_CreateAndShowInstance, reinterpret_cast<jlong>(this), activity->clazz,
      env->NewStringUTF(m_titleText), env->NewStringUTF(m_text), env->NewStringUTF(m_buttonText[0]), env->NewStringUTF(m_buttonText[1]), env->NewStringUTF(m_buttonText[2]));

    if (env->ExceptionOccurred())
    {
      env->ExceptionDescribe();
      env->ExceptionClear();
      return;
    }

    jvm->DetachCurrentThread();

    while(!m_dialog)
    {
      PollAndroidOnce();
      Sleep(10);
    }
  }

  virtual void Close() HKV_OVERRIDE
  {
    if(m_dialog)
    {
      ANativeActivity* activity = AndroidApplication->activity; 

      JNIEnv *env = 0;
      JavaVM *jvm = activity->vm;
      jvm->AttachCurrentThread(&env, NULL);

      jmethodID method_VNativeMobileDialogAndroid_CloseInstance = env->GetStaticMethodID(m_clazz_Dialog, "CloseInstance",
        "(Lcom/havok/Vision/VNativeMobileDialogAndroid;)V");
      CHECK_SUCCESS(method_VNativeMobileDialogAndroid_CloseInstance);

      env->CallStaticVoidMethod(m_clazz_Dialog, method_VNativeMobileDialogAndroid_CloseInstance, m_dialog);

      env->DeleteGlobalRef(m_dialog);

      jvm->DetachCurrentThread();

      m_dialog = NULL;
    }
  }

  virtual State GetState()
  {
    return m_eState;
  }

  jclass m_clazz_Dialog;
  jobject m_dialog;

  State m_eState;

  VString m_titleText;
  VString m_text;
  VString m_buttonText[3];
  int m_iNumButtons;
};

IVNativeMobileDialog* IVNativeMobileDialog::CreateInstance()
{
  return new VNativeMobileDialogAndroid();
}

extern "C" JNIEXPORT void JNICALL Java_com_havok_Vision_VNativeMobileDialogAndroid_SetDialogState(JNIEnv* env, jclass clazz, jlong nativePointer, jint which)
{
  reinterpret_cast<VNativeMobileDialogAndroid*>(nativePointer)->m_eState = static_cast<IVNativeMobileDialog::State>(which);
}

extern "C" JNIEXPORT void JNICALL Java_com_havok_Vision_VNativeMobileDialogAndroid_SetDialogInstance(JNIEnv* env, jclass clazz, jlong nativePointer, jobject dialog)
{
  reinterpret_cast<VNativeMobileDialogAndroid*>(nativePointer)->m_dialog = env->NewGlobalRef(dialog);
}

/*
 * Havok SDK - Base file, BUILD(#20131019)
 * 
 * Confidential Information of Havok.  (C) Copyright 1999-2013
 * Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
 * Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
 * rights, and intellectual property rights in the Havok software remain in
 * Havok and/or its suppliers.
 * 
 * Use of this software for evaluation purposes is subject to and indicates
 * acceptance of the End User licence Agreement for this product. A copy of
 * the license is included with this software and is also available from salesteam@havok.com.
 * 
 */
