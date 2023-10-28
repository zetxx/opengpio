#include <napi.h>
#include <iostream>
#include <gpiod.hpp>
#include <unistd.h>
using namespace std;

Napi::Array GpioInput(Napi::CallbackInfo const& info){
    Napi::Env env = info.Env();

    int chipNumber = info[0].As<Napi::Number>().Int32Value();
    int lineNumber = info[1].As<Napi::Number>().Int32Value();

    ::gpiod::chip chip("gpiochip" + to_string(chipNumber));
    auto line = chip.get_line(lineNumber);
    line.request({"opengpio", gpiod::line_request::DIRECTION_INPUT, 0});  

    Napi::Function getter = Napi::Function::New(info.Env(), [line](const Napi::CallbackInfo &info){
        bool value = line.get_value();
        return Napi::Boolean::New(info.Env(), value); 
    });

    Napi::Function release = Napi::Function::New(info.Env(), [line](const Napi::CallbackInfo &info){
        line.release();
    });

    Napi::Array arr = Napi::Array::New(info.Env(), 2);
    arr.Set(0u, getter);
    arr.Set(1u, release);

    return arr;
}

Napi::Array GpioOutput(Napi::CallbackInfo const& info){
    // Napi::Env env = info.Env();
    int chipNumber = info[0].As<Napi::Number>().Int32Value();
    int lineNumber = info[1].As<Napi::Number>().Int32Value();

    ::gpiod::chip chip("gpiochip" + to_string(chipNumber));
    auto line = chip.get_line(lineNumber);
    line.request({"opengpio", gpiod::line_request::DIRECTION_OUTPUT, 0}, 1);  

    Napi::Function setter = Napi::Function::New(info.Env(), [line](const Napi::CallbackInfo &info){
        bool value = info[0].As<Napi::Boolean>().ToBoolean();
        line.set_value(value);
    });

    Napi::Function release = Napi::Function::New(info.Env(), [line](const Napi::CallbackInfo &info){
        line.release();
    });

    Napi::Array arr = Napi::Array::New(info.Env(), 2);
    arr.Set(0u, setter);
    arr.Set(1u, release);

    return arr;
}

void GpioWatch(Napi::CallbackInfo const& info){
}

void GpioPwm(Napi::CallbackInfo const& info){
    // Napi::Env env = info.Env();

    int chipNumber = info[0].As<Napi::Number>().Int32Value();
    int lineNumber = info[1].As<Napi::Number>().Int32Value();
    double dutyCycle = info[2].As<Napi::Number>().DoubleValue();
    int frequency = info[3].As<Napi::Number>().Int32Value();


    printf("Pwm GPIO at: %d:%d -> %f:%d\n", chipNumber, lineNumber, dutyCycle, frequency);

    // Calculating sleep times in microseconds
    double period = 1.0 / frequency;
    double on_time = period * dutyCycle;
    double off_time = period - on_time;
    int on_time_us = on_time * 1e6;
    int off_time_us = off_time * 1e6;


    ::gpiod::chip chip("gpiochip" + to_string(chipNumber));
    auto line = chip.get_line(lineNumber);
    line.request({"opengpio", gpiod::line_request::DIRECTION_OUTPUT, 0}, 1);

    while (1) { // Continuous loop to generate PWM  
        line.set_value(true);
        usleep(on_time_us);
        line.set_value(false);
        usleep(off_time_us);
    }

    line.release();
}

Napi::String Info(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::String::New(env, "Built for the wanderers, the explorers, the adventurers of the world. For those who want to see the world, not read about it. For those who want to get out there and experience it all. For those who want to live their dreams, not sleep through them. For those who want to see the world their way. Adventure belongs to the couragous. Exploration Systems.");
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports["info"] = Napi::Function::New(env, Info);
  exports["input"] = Napi::Function::New(env, GpioInput);
  exports["output"] = Napi::Function::New(env, GpioOutput);
  exports["watch"] = Napi::Function::New(env, GpioWatch);
  exports["pwm"] = Napi::Function::New(env, GpioPwm);
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)