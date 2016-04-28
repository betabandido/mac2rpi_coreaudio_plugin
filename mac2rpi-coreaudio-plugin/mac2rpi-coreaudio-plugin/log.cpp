#include "log.h"

#include <syslog.h>

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

// TODO remove

#include "types.h"

std::string ObjectIDToString(AudioObjectID object_id) {
  switch (object_id) {
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
      return "property_base_class";
    case kAudioObjectPropertyClass:
      return "property_class";
    case kAudioObjectPropertyOwner:
      return "property_owner";
    case kAudioObjectPropertyManufacturer:
      return "property_manufacturer";
    case kAudioObjectPropertyOwnedObjects:
      return "property_owned_objects";
    case kAudioObjectPropertyName:
      return "property_name";
    case kAudioObjectPropertyModelName:
      return "property_model_name";
    case kAudioObjectPropertyIdentify:
      return "property_identify";
    case kAudioObjectPropertySerialNumber:
      return "property_serial_number";
    case kAudioObjectPropertyFirmwareVersion:
      return "property_firmware_version";
  }
  
  return boost::str(boost::format("### unknown (%1%) ###") % object_id);
}

std::string PluginPropertyToString(AudioObjectID object_id) {
  switch (object_id) {
    case kAudioPlugInPropertyBoxList:
      return "property_box_list";
    case kAudioPlugInPropertyTranslateUIDToBox:
      return "property_translate_uid_to_box";
    case kAudioPlugInPropertyDeviceList:
      return "property_device_list";
    case kAudioPlugInPropertyTranslateUIDToDevice:
      return "property_translate_uid_to_device";
//    case kAudioPlugInPropertyResourceBundle:
//      return "property_resource_bundle";
  }
  
  return ObjectIDToString(object_id);
}

std::string DevicePropertyToString(AudioObjectID object_id) {
  switch (object_id) {
    case kAudioDevicePropertyDeviceUID:
      return "device_property_device_uid";
    case kAudioDevicePropertyModelUID:
      return "device_property_model_uid";
    case kAudioDevicePropertyTransportType:
      return "device_property_transport_type";
    case kAudioDevicePropertyRelatedDevices:
      return "device_property_related_devices";
    case kAudioDevicePropertyClockDomain:
      return "device_property_clock_domain";
    case kAudioDevicePropertyDeviceIsAlive:
      return "device_property_device_is_alive";
    case kAudioDevicePropertyDeviceIsRunning:
      return "device_property_device_is_running";
    case kAudioObjectPropertyControlList:
      return "device_property_control_list";
    case kAudioDevicePropertyNominalSampleRate:
      return "device_property_nominal_sample_rates";
    case kAudioDevicePropertyAvailableNominalSampleRates:
      return "device_property_available_nominal_sample_rates";
    case kAudioDevicePropertyIsHidden:
      return "device_property_is_hidden";
//    case kAudioDevicePropertyZeroTimeStampPeriod:
//      return "device_property_zero_time_stamp_period";
    case kAudioDevicePropertyIcon:
      return "device_property_icon";
    case kAudioDevicePropertyStreams:
      return "device_property_streams";
    case kAudioDevicePropertyDeviceCanBeDefaultDevice:
      return "device_property_can_be_default_device";
    case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
      return "device_property_can_be_default_system_device";
    case kAudioDevicePropertyLatency:
      return "device_property_latency";
    case kAudioDevicePropertySafetyOffset:
      return "device_property_safety_offset";
    case kAudioDevicePropertyPreferredChannelsForStereo:
      return "device_property_preferred_channels_for_stereo";
    case kAudioDevicePropertyPreferredChannelLayout:
      return "device_property_preferred_channel_layout";
  }
  
  return ObjectIDToString(object_id);
}

std::string StreamPropertyToString(AudioObjectID object_id) {
  switch (object_id) {
    case kAudioStreamPropertyIsActive:
      return "stream_property_is_active";
    case kAudioStreamPropertyDirection:
      return "stream_property_direction";
    case kAudioStreamPropertyTerminalType:
      return "stream_property_terminal_type";
    case kAudioStreamPropertyStartingChannel:
      return "stream_property_starting_channel";
    case kAudioStreamPropertyLatency:
      return "stream_property_latency";
    case kAudioStreamPropertyVirtualFormat:
      return "stream_property_virtual_format";
    case kAudioStreamPropertyPhysicalFormat:
      return "stream_property_physical_format";
    case kAudioStreamPropertyAvailableVirtualFormats:
      return "stream_property_available_virtual_formats";
    case kAudioStreamPropertyAvailablePhysicalFormats:
      return "stream_property_available_physical_formats";
  }
  
  return ObjectIDToString(object_id);
}

std::string ControlPropertyToString(AudioObjectID object_id) {
  switch (object_id) {
    case kAudioControlPropertyScope:
      return "control_property_scope";
    case kAudioControlPropertyElement:
      return "control_property_element";
    case kAudioLevelControlPropertyScalarValue:
      return "level_control_property_scalar_value";
    case kAudioLevelControlPropertyDecibelValue:
      return "level_control_property_decibel_value";
    case kAudioLevelControlPropertyDecibelRange:
      return "level_control_property_decibel_range";
    case kAudioLevelControlPropertyConvertScalarToDecibels:
      return "level_control_property_convert_scalar_to_decibels";
    case kAudioLevelControlPropertyConvertDecibelsToScalar:
      return "level_control_property_convert_decibels_to_scalar";
    case kAudioBooleanControlPropertyValue:
      return "boolean_control_property_value";
    case kAudioSelectorControlPropertyCurrentItem:
      return "selector_control_property_current_item";
    case kAudioSelectorControlPropertyAvailableItems:
      return "selector_control_property_available_items";
    case kAudioSelectorControlPropertyItemName:
      return "selector_control_property_item_name";
  }
  
  return ObjectIDToString(object_id);
}
