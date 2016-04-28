#include "log.h"

#include <syslog.h>

#include "types.h"

void log(const std::string& s) noexcept {
  try {
    syslog(LOG_NOTICE, "%s", s.c_str());
  } catch (...) {}
}

void log(const boost::format& fmt) noexcept {
  try {
    log(boost::str(fmt));
  } catch (...) {}
}

std::string ObjectIDToString(AudioObjectID objectID) {
  switch (objectID) {
    case kObjectID_PlugIn:
      return "plugin";
    case kObjectID_Device:
      return "device";
    case kObjectID_Stream_Output:
      return "stream_output";
    case kObjectID_Volume_Output_Master:
      return "volume_output_master";
    case kObjectID_Mute_Output_Master:
      return "mute_output_master";
    case kAudioObjectPropertyBaseClass:
      return "base_class";
    case kAudioObjectPropertyClass:
      return "class";
    case kAudioObjectPropertyOwner:
      return "owner";
    case kAudioObjectPropertyName:
      return "name";
    case kAudioObjectPropertyModelName:
      return "model_name";
    case kAudioObjectPropertyManufacturer:
      return "manufacturer";
    case kAudioObjectPropertyOwnedObjects:
      return "owned_objects";
    case kAudioObjectPropertyIdentify:
      return "identify";
    case kAudioObjectPropertySerialNumber:
      return "serial_number";
    case kAudioObjectPropertyFirmwareVersion:
      return "firmware_version";
  }
  
  return boost::str(boost::format("### unknown (%1%) ###") % objectID);
}

std::string PluginPropertyToString(AudioObjectID objectID) {
  switch (objectID) {
    case kAudioPlugInPropertyDeviceList:
      return "device_list";
    case kAudioPlugInPropertyTranslateUIDToDevice:
      return "translate_uid_to_device";
    case kAudioPlugInPropertyBoxList:
      return "box_list";
    case kAudioPlugInPropertyTranslateUIDToBox:
      return "translate_uid_to_box";
    case kAudioPlugInPropertyResourceBundle:
      return "resource_bundle";
  }
  
  return ObjectIDToString(objectID);
}

std::string DevicePropertyToString(AudioObjectID objectID) {
  switch (objectID) {
    case kAudioDevicePropertyDeviceUID:
      return "device_device_uid";
    case kAudioDevicePropertyModelUID:
      return "device_model_uid";
    case kAudioDevicePropertyTransportType:
      return "device_transport_type";
    case kAudioDevicePropertyRelatedDevices:
      return "device_related_devices";
    case kAudioDevicePropertyClockDomain:
      return "device_clock_domain";
    case kAudioDevicePropertyDeviceIsAlive:
      return "device_device_is_alive";
    case kAudioDevicePropertyDeviceIsRunning:
      return "device_device_is_running";
    case kAudioDevicePropertyDeviceCanBeDefaultDevice:
      return "device_can_be_default_device";
    case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
      return "device_can_be_default_system_device";
    case kAudioDevicePropertyLatency:
      return "device_latency";
    case kAudioDevicePropertyStreams:
      return "device_streams";
    case kAudioObjectPropertyControlList:
      return "device_control_list";
    case kAudioDevicePropertySafetyOffset:
      return "device_safety_offset";
    case kAudioDevicePropertyNominalSampleRate:
      return "device_nominal_sample_rates";
    case kAudioDevicePropertyAvailableNominalSampleRates:
      return "device_available_nominal_sample_rates";
    case kAudioDevicePropertyIcon:
      return "device_icon";
    case kAudioDevicePropertyIsHidden:
      return "device_is_hidden";
    case kAudioDevicePropertyPreferredChannelsForStereo:
      return "device_preferred_channels_for_stereo";
    case kAudioDevicePropertyPreferredChannelLayout:
      return "device_preferred_channel_layout";
    case kAudioDevicePropertyZeroTimeStampPeriod:
      return "device_zero_time_stamp_period";
  }
  
  return ObjectIDToString(objectID);
}

std::string StreamPropertyToString(AudioObjectID objectID) {
  switch (objectID) {
    case kAudioStreamPropertyIsActive:
      return "stream_is_active";
    case kAudioStreamPropertyDirection:
      return "stream_direction";
    case kAudioStreamPropertyTerminalType:
      return "stream_terminal_type";
    case kAudioStreamPropertyStartingChannel:
      return "stream_starting_channel";
    case kAudioStreamPropertyLatency:
      return "stream_latency";
    case kAudioStreamPropertyVirtualFormat:
      return "stream_virtual_format";
    case kAudioStreamPropertyAvailableVirtualFormats:
      return "stream_available_virtual_formats";
    case kAudioStreamPropertyPhysicalFormat:
      return "stream_physical_format";
    case kAudioStreamPropertyAvailablePhysicalFormats:
      return "stream_available_physical_formats";
  }
  
  return ObjectIDToString(objectID);
}

std::string ControlPropertyToString(AudioObjectID objectID) {
  switch (objectID) {
    case kAudioControlPropertyScope:
      return "control_scope";
    case kAudioControlPropertyElement:
      return "control_element";
    case kAudioLevelControlPropertyScalarValue:
      return "level_control_scalar_value";
    case kAudioLevelControlPropertyDecibelValue:
      return "level_control_decibel_value";
    case kAudioLevelControlPropertyDecibelRange:
      return "level_control_decibel_range";
    case kAudioLevelControlPropertyConvertScalarToDecibels:
      return "level_control_convert_scalar_to_decibels";
    case kAudioLevelControlPropertyConvertDecibelsToScalar:
      return "level_control_convert_decibels_to_scalar";
    case kAudioBooleanControlPropertyValue:
      return "boolean_control_value";
    case kAudioSelectorControlPropertyCurrentItem:
      return "selector_control_current_item";
    case kAudioSelectorControlPropertyAvailableItems:
      return "selector_control_available_items";
    case kAudioSelectorControlPropertyItemName:
      return "selector_control_item_name";
    case kAudioSelectorControlPropertyItemKind:
      return "selector_control_item_kind";
  }
  
  return ObjectIDToString(objectID);
}
