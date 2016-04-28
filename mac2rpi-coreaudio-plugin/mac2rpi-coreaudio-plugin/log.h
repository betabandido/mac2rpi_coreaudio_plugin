#ifndef log_h
#define log_h

// TODO remove
#define DEBUG 1

#include <string>
#include <boost/format.hpp>

#ifdef DEBUG
  #define LOG(x) log(x)
#else
  #define LOG(x) do {} while (false)
#endif

void log(const std::string& s) noexcept;

void log(const boost::format& fmt) noexcept;

// TODO remove
#include <string>
#include <CoreAudio/AudioDriverPlugIn.h>

std::string ObjectIDToString(AudioObjectID object_id);
std::string PluginPropertyToString(AudioObjectID object_id);
std::string DevicePropertyToString(AudioObjectID object_id);
std::string StreamPropertyToString(AudioObjectID object_id);
std::string ControlPropertyToString(AudioObjectID object_id);

#endif /* log_h */
