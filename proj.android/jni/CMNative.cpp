#include <jni.h>
#include <stdio.h>
#include <android/log.h>
#include "../../breakpad/src/client/linux/handler/exception_handler.h"
#include "../../breakpad/src/client/linux/handler/minidump_descriptor.h"

static google_breakpad::ExceptionHandler* exceptionHandler;
bool DumpCallback(const google_breakpad::MinidumpDescriptor& descriptor,
                  void* context,
                  bool succeeded) {
  __android_log_print(ANDROID_LOG_DEBUG, "BREAKPAD", "Dump path: %s\n", descriptor.path());
  return succeeded;
}

void Crash() {
  volatile int* a = reinterpret_cast<volatile int*>(NULL);
  *a = 1;
}

extern "C" {

void Java_com_cheekymammoth_puzzlewizard_puzzlewizard_setUpBreakpad(JNIEnv* env, jobject obj, jstring filepath) {
  const char *path = env->GetStringUTFChars(filepath, 0);
  google_breakpad::MinidumpDescriptor descriptor(path);
  exceptionHandler = new google_breakpad::ExceptionHandler(descriptor, NULL, DumpCallback, NULL, true, -1);
  __android_log_print(ANDROID_LOG_DEBUG, "BREAKPAD", "Breakpad setup complete with dump path: %s\n", path);
}

void Java_com_cheekymammoth_puzzlewizard_puzzlewizard_nativeCrash(JNIEnv* env, jobject obj) {
  Crash();
}

} // end extern "C"
