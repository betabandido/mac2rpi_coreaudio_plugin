#ifndef OSException_h
#define OSException_h

#include <stdexcept>

#include <MacTypes.h>

class OSException : public std::runtime_error {
public:
  OSException(const std::string& what_arg,
              OSStatus status = kAudioHardwareUnspecifiedError)
  : std::runtime_error(what_arg)
  , status_{status}
  {}

  OSException(const char* what_arg,
              OSStatus status = kAudioHardwareUnspecifiedError)
  : std::runtime_error(what_arg)
  , status_{status}
  {}
  
  const OSStatus status() const {
    return status_;
  }
  
private:
  OSStatus status_;
};

#endif /* OSException_h */
