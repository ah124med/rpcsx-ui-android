#include <jni.h>
#include <android/log.h>
#include <thread>
#include "rpcsx/rpcsx.h" // Assume RPCSX API exists

#define LOG_TAG "NativeRPCSX"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Use affinity and optimize scheduling on big cores
#ifdef __ANDROID__
#include <sched.h>
#include <unistd.h>

#void set_high_performance_core()
#ifdef __ANDROID__
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    // Set to use big cores only (e.g., CPU 4–7 on Snapdragon)
    for (int i = 4; i < 8; i++) CPU_SET(i, &cpuset);
    sched_setaffinity(0, sizeof(cpuset), &cpuset);

extern "C"
JNIEXPORT void JNICALL
Java_com_example_rpcsx_NativeBridge_init(JNIEnv* env, jobject /* this */) {
    LOGI("RPCSX initializing...");

    set_high_performance_core();

    rpcsx::InitConfig config{};
    config.enable_jit = true;
    config.use_gpu_renderer = true;
    config.system_arch = rpcsx::SystemArch::ARM64;

    if (!rpcsx::initialize(config)) {
        LOGI("RPCSX initialization failed");
        return;
    }
    LOGI("RPCSX initialized on high-performance core.");
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_rpcsx_NativeBridge_runFrame(JNIEnv* env, jobject /* this */) {
    set_high_performance_core(); // Ensure frame runs on big core
    rpcsx::run_frame();
}
