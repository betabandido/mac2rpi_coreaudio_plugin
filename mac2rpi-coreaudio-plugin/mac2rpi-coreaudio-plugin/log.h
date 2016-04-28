#ifndef log_h
#define log_h

// TODO remove
#define DEBUG 1

#ifdef DEBUG
  #include <string>
  #include <boost/format.hpp>
  #include <CoreAudio/AudioDriverPlugIn.h>

  /** Logs a message in the system log.
   *
   * @param s The string containing the message.
   */
  void log(const std::string& s) noexcept;

  /** Logs a message in the system log.
   *
   * @param fmt The formatting object containing the message.
   */
  void log(const boost::format& fmt) noexcept;

  std::string ObjectIDToString(AudioObjectID objectID);
  std::string PluginPropertyToString(AudioObjectID objectID);
  std::string DevicePropertyToString(AudioObjectID objectID);
  std::string StreamPropertyToString(AudioObjectID objectID);
  std::string ControlPropertyToString(AudioObjectID objectID);

  #define LOG(x) log(x)
#else
  #define LOG(x) do {} while (false)
#endif

#endif /* log_h */
