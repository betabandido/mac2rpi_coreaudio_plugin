#include "Device.h"

#include <algorithm>
#include <numeric>

#include <mach/mach_time.h>

#include "Control.h"
#include "log.h"
#include "OSException.h"
#include "Stream.h"
#include "types.h"

namespace asio = boost::asio;
using boost::asio::ip::tcp;

constexpr std::array<Float64, 2> Device::availableSampleRates;
constexpr Float32 Device::volumeMinDB;
constexpr Float32 Device::volumeMaxDB;
constexpr unsigned Device::numberOfStreams;
constexpr unsigned Device::numberOfControls;
constexpr unsigned Device::numberOfSubObjects;
constexpr unsigned Device::numberOfChannels;
constexpr unsigned Device::ringBufferSize;

Device::Device()
  : AudioObject(kObjectID_Device,
              kAudioDeviceClassID,
              kAudioObjectClassID,
              kObjectID_PlugIn)
  , outputStream_(std::make_shared<Stream>
                  (kObjectID_Stream_Output, *this))
  , volumeControl_(std::make_shared<VolumeControl>
                   (kObjectID_Volume_Output_Master, *this))
  , muteControl_(std::make_shared<MuteControl>
                 (kObjectID_Mute_Output_Master, *this))
  , outputSocket_(std::make_unique<TCPSocket>(ioService_))
{
  AudioObjectMap::AddObject(kObjectID_Stream_Output, outputStream_);
  AudioObjectMap::AddObject(kObjectID_Volume_Output_Master, volumeControl_);
  AudioObjectMap::AddObject(kObjectID_Mute_Output_Master, muteControl_);
}

Boolean Device::HasProperty(pid_t clientProcessID,
                            const AudioObjectPropertyAddress& address) const {

  LOG(boost::format("DeviceHasProperty: selector=%1%")
      % DevicePropertyToString(address.mSelector));

  switch (address.mSelector) {
    case kAudioObjectPropertyName:
    case kAudioObjectPropertyManufacturer:
    case kAudioDevicePropertyDeviceUID:
    case kAudioDevicePropertyModelUID:
    case kAudioDevicePropertyTransportType:
    case kAudioDevicePropertyRelatedDevices:
    case kAudioDevicePropertyClockDomain:
    case kAudioDevicePropertyDeviceIsAlive:
    case kAudioDevicePropertyDeviceIsRunning:
    case kAudioObjectPropertyControlList:
    case kAudioDevicePropertyNominalSampleRate:
    case kAudioDevicePropertyAvailableNominalSampleRates:
    case kAudioDevicePropertyIsHidden:
    case kAudioDevicePropertyZeroTimeStampPeriod:
    case kAudioDevicePropertyStreams:
      return true;
      
    case kAudioDevicePropertyLatency:
    case kAudioDevicePropertySafetyOffset:
    case kAudioDevicePropertyPreferredChannelsForStereo:
    case kAudioDevicePropertyPreferredChannelLayout:
    case kAudioDevicePropertyDeviceCanBeDefaultDevice:
    case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
      return (address.mScope == kAudioObjectPropertyScopeOutput);
  }
  
  return AudioObject::HasProperty(clientProcessID, address);
}

Boolean Device::IsPropertySettable(pid_t clientProcessID,
                                   const AudioObjectPropertyAddress& address) const {
  switch (address.mSelector) {
    case kAudioObjectPropertyName:
    case kAudioObjectPropertyManufacturer:
    case kAudioDevicePropertyDeviceUID:
    case kAudioDevicePropertyModelUID:
    case kAudioDevicePropertyTransportType:
    case kAudioDevicePropertyRelatedDevices:
    case kAudioDevicePropertyClockDomain:
    case kAudioDevicePropertyDeviceIsAlive:
    case kAudioDevicePropertyDeviceIsRunning:
    case kAudioDevicePropertyDeviceCanBeDefaultDevice:
    case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
    case kAudioDevicePropertyLatency:
    case kAudioDevicePropertyStreams:
    case kAudioObjectPropertyControlList:
    case kAudioDevicePropertySafetyOffset:
    case kAudioDevicePropertyAvailableNominalSampleRates:
    case kAudioDevicePropertyIsHidden:
    case kAudioDevicePropertyPreferredChannelsForStereo:
    case kAudioDevicePropertyPreferredChannelLayout:
    case kAudioDevicePropertyZeroTimeStampPeriod:
      return false;
      
    case kAudioDevicePropertyNominalSampleRate:
      return true;
  };
  
  return AudioObject::IsPropertySettable(clientProcessID, address);
}

UInt32 Device::GetPropertyDataSize(pid_t clientProcessID,
                                   const AudioObjectPropertyAddress& address,
                                   UInt32 qualifierDataSize,
                                   const void* qualifierData) const {
  switch (address.mSelector) {
    case kAudioObjectPropertyName:
      return sizeof(CFStringRef);
      
    case kAudioObjectPropertyManufacturer:
      return sizeof(CFStringRef);
      
    case kAudioObjectPropertyOwnedObjects:
      return (address.mScope == kAudioObjectPropertyScopeGlobal
              || address.mScope == kAudioObjectPropertyScopeOutput)
          ? numberOfSubObjects * sizeof(AudioObjectID)
          : 0;
      
    case kAudioDevicePropertyDeviceUID:
    case kAudioDevicePropertyModelUID:
      return sizeof(CFStringRef);
      
    case kAudioDevicePropertyTransportType:
      return sizeof(UInt32);
      
    case kAudioDevicePropertyRelatedDevices:
      return sizeof(AudioObjectID);
      
    case kAudioDevicePropertyClockDomain:
      return sizeof(UInt32);
      
    case kAudioDevicePropertyDeviceIsAlive:
      return sizeof(AudioClassID);
      
    case kAudioDevicePropertyDeviceIsRunning:
    case kAudioDevicePropertyDeviceCanBeDefaultDevice:
    case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
    case kAudioDevicePropertyLatency:
      return sizeof(UInt32);
      
    case kAudioDevicePropertyStreams:
      return (address.mScope == kAudioObjectPropertyScopeGlobal
              || address.mScope == kAudioObjectPropertyScopeOutput)
      ? numberOfStreams * sizeof(AudioObjectID)
      : 0;
      
    case kAudioObjectPropertyControlList:
      return numberOfControls * sizeof(AudioObjectID);
      
    case kAudioDevicePropertySafetyOffset:
      return sizeof(UInt32);
      
    case kAudioDevicePropertyNominalSampleRate:
      return sizeof(Float64);
      
    case kAudioDevicePropertyAvailableNominalSampleRates:
      return availableSampleRates.size() * sizeof(AudioValueRange);
      
    case kAudioDevicePropertyIsHidden:
      return sizeof(UInt32);
      
    case kAudioDevicePropertyPreferredChannelsForStereo:
      return numberOfChannels * sizeof(UInt32);
      
    case kAudioDevicePropertyPreferredChannelLayout:
      return offsetof(AudioChannelLayout, mChannelDescriptions)
          + (numberOfChannels * sizeof(AudioChannelDescription));
      
    case kAudioDevicePropertyZeroTimeStampPeriod:
      return sizeof(UInt32);
  };
  
  return AudioObject::GetPropertyDataSize(clientProcessID,
                                          address,
                                          qualifierDataSize,
                                          qualifierData);
}

UInt32 Device::GetPropertyData(pid_t clientProcessID,
                               const AudioObjectPropertyAddress& address,
                               UInt32 qualifierDataSize,
                               const void* qualifierData,
                               UInt32 dataSize,
                               void* data) const {
  
  LOG(boost::format("DeviceGetPropertyData: selector=%1%")
      % DevicePropertyToString(address.mSelector));

  switch (address.mSelector) {
    case kAudioObjectPropertyName:
      return GetPropertyDataImpl<CFStringRef>
          (dataSize, CFSTR("mac2rpi-plugin"), data);
      
    case kAudioObjectPropertyManufacturer:
      return GetPropertyDataImpl<CFStringRef>
          (dataSize, CFSTR("mac2rpi"), data);
      
    case kAudioObjectPropertyOwnedObjects:
    {
      UInt32 itemCount = dataSize / sizeof(AudioObjectID);
      if (address.mScope == kAudioObjectPropertyScopeGlobal
          || address.mScope == kAudioObjectPropertyScopeOutput) {
        itemCount = std::min(itemCount, numberOfSubObjects);
        auto objData = static_cast<AudioObjectID*>(data);
        std::iota(objData,
                  objData + itemCount,
                  static_cast<AudioObjectID>(kObjectID_Stream_Output));
        return itemCount * sizeof(AudioObjectID);
      }
      return 0;
    }
      
    case kAudioDevicePropertyDeviceUID:
      return GetPropertyDataImpl<CFStringRef>
          (dataSize, CFSTR("mac2rpi-device"), data);
      
    case kAudioDevicePropertyModelUID:
      return GetPropertyDataImpl<CFStringRef>
          (dataSize, CFSTR("mac2rpi-device"), data);
      
    case kAudioDevicePropertyTransportType:
      return GetPropertyDataImpl<UInt32>
          (dataSize, kAudioDeviceTransportTypeVirtual, data);
      
    case kAudioDevicePropertyRelatedDevices:
    {
      UInt32 itemCount = dataSize / sizeof(AudioObjectID);
      // a device is related to itself
      itemCount = std::min(itemCount, 1U);
      if (itemCount > 0) {
        auto objData = static_cast<AudioObjectID*>(data);
        objData[0] = ObjectID();
        return itemCount * sizeof(AudioObjectID);
      }
      return 0;
    }
      
    case kAudioDevicePropertyClockDomain:
      return GetPropertyDataImpl<UInt32>(dataSize, 0, data);
      
    case kAudioDevicePropertyDeviceIsAlive:
      return GetPropertyDataImpl<UInt32>(dataSize, 1, data);
      
    case kAudioDevicePropertyDeviceIsRunning:
      return GetPropertyDataImpl<UInt32>
          (dataSize, (ioIsRunning_ > 0), data);
      
    case kAudioDevicePropertyDeviceCanBeDefaultDevice:
      return GetPropertyDataImpl<UInt32>(dataSize, 1, data);
      
    case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
      return GetPropertyDataImpl<UInt32>(dataSize, 1, data);
      
    case kAudioDevicePropertyLatency:
      return GetPropertyDataImpl<UInt32>(dataSize, 0, data);
      
    case kAudioDevicePropertyStreams:
    {
      UInt32 itemCount = dataSize / sizeof(AudioObjectID);
      itemCount = std::min(itemCount, numberOfStreams);
      if (address.mScope == kAudioObjectPropertyScopeGlobal
          || address.mScope == kAudioObjectPropertyScopeOutput) {
        if (itemCount > 0) {
          static_assert(numberOfStreams == 1, "Wrong number of streams");
          auto objData = static_cast<AudioObjectID*>(data);
          objData[0] = kObjectID_Stream_Output;
          return sizeof(AudioObjectID);
        }
      }
      return 0;
    }
      
    case kAudioObjectPropertyControlList:
    {
      UInt32 itemCount = dataSize / sizeof(AudioObjectID);
      itemCount = std::min(itemCount, numberOfControls);
      if (address.mScope == kAudioObjectPropertyScopeGlobal
          || address.mScope == kAudioObjectPropertyScopeOutput) {
        auto objData = static_cast<AudioObjectID*>(data);
        std::iota(objData,
                  objData + itemCount,
                  static_cast<AudioObjectID>(kObjectID_Volume_Output_Master));
        return itemCount * sizeof(AudioObjectID);
      }
      return 0;
    }
      
    case kAudioDevicePropertySafetyOffset:
      return GetPropertyDataImpl<UInt32>(dataSize, 0, data);
      
    case kAudioDevicePropertyNominalSampleRate:
      return GetPropertyDataImpl<Float64>(dataSize, sampleRate_, data);
      
    case kAudioDevicePropertyAvailableNominalSampleRates:
    {
      UInt32 itemCount = dataSize / sizeof(AudioValueRange);
      itemCount = std::min<UInt32>(itemCount, availableSampleRates.size());
      auto valueRange = static_cast<AudioValueRange*>(data);
      for (unsigned i = 0; i < itemCount; i++) {
        valueRange[i].mMinimum = availableSampleRates[i];
        valueRange[i].mMaximum = availableSampleRates[i];
      }
      return itemCount * sizeof(AudioValueRange);
    }

    case kAudioDevicePropertyIsHidden:
      return GetPropertyDataImpl<UInt32>(dataSize, 0, data);
      
    case kAudioDevicePropertyPreferredChannelsForStereo:
      static_assert(numberOfChannels == 2, "Wrong number of channels");
      CheckOutDataSize(dataSize, numberOfChannels * sizeof(UInt32));
      static_cast<UInt32*>(data)[0] = 1;
      static_cast<UInt32*>(data)[1] = 2;
      return numberOfChannels * sizeof(UInt32);
      
    case kAudioDevicePropertyPreferredChannelLayout:
      // Return a stereo ACL.
    {
      UInt32 ACLsize = offsetof(AudioChannelLayout, mChannelDescriptions)
          + (numberOfChannels * sizeof(AudioChannelDescription));
      CheckOutDataSize(dataSize, ACLsize);
      auto ACLdata = static_cast<AudioChannelLayout*>(data);
      ACLdata->mChannelLayoutTag = kAudioChannelLayoutTag_UseChannelDescriptions;
      ACLdata->mChannelBitmap = 0;
      ACLdata->mNumberChannelDescriptions = numberOfChannels;
      
      for (unsigned i = 0; i < numberOfChannels; i++) {
        auto& desc = ACLdata->mChannelDescriptions[i];
        desc.mChannelLabel = kAudioChannelLabel_Left + i;
        desc.mChannelFlags = 0;
        desc.mCoordinates[0] = 0;
        desc.mCoordinates[1] = 0;
        desc.mCoordinates[2] = 0;
      }
        
      return ACLsize;
    }
      
    case kAudioDevicePropertyZeroTimeStampPeriod:
      return GetPropertyDataImpl<UInt32>
          (dataSize, ringBufferSize, data);
  };
  
  return AudioObject::GetPropertyData(clientProcessID,
                                      address,
                                      qualifierDataSize,
                                      qualifierData,
                                      dataSize,
                                      data);
}

std::pair<UInt32, Device::ChangedPropertyList>
Device::SetPropertyData(pid_t clientProcessID,
                        const AudioObjectPropertyAddress& address,
                        UInt32 qualifierDataSize,
                        const void* qualifierData,
                        UInt32 dataSize,
                        const void* data) {
  switch (address.mSelector) {
    case kAudioDevicePropertyNominalSampleRate:
    {
      CheckInDataSize(dataSize, sizeof(Float64));
      
      LOG("############## Set nominal sample rate: UNSUPPORTED");
      // TODO
#if 0
      auto sampleRate = *(static_cast<const Float64*>(data));
      if (sampleRate != 44100.0 && sampleRate != 48000.0)
        throw OSException("invalid sample rate",
                          kAudioHardwareIllegalOperationError);
      
      LOG(boost::format("Changing sample rate: %1%") % sampleRate);
      
      // XXX can't we just use atomics?
      Float64 oldSampleRate;
      {
      std::lock_guard<std::mutex> lock(gPlugInStateMutex);
      oldSampleRate = gDeviceSampleRate;
      }
      
      if (sampleRate != oldSampleRate) {
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
          ^{ gPlugInHost->RequestDeviceConfigurationChange(gPluginHost,
                                                           kObjectID_Device,
                                                           static_cast<UInt64>(sampleRate),
                                                           nullptr);
          });
      }
#endif
      
    }
  };
  
  return AudioObject::SetPropertyData(clientProcessID,
                                      address,
                                      qualifierDataSize,
                                      qualifierData,
                                      dataSize,
                                      data);
}

void Device::ComputeHostTicksPerFrame() {
  struct mach_timebase_info timeBaseInfo;
  mach_timebase_info(&timeBaseInfo);
  auto hostClockFrequency =
  static_cast<Float64>(timeBaseInfo.denom) / timeBaseInfo.numer;
  hostClockFrequency *= 1000000000.0;
  hostTicksPerFrame_ = hostClockFrequency / sampleRate_;
  LOG(boost::format("###### host ticks per frame: %1%") % hostTicksPerFrame_);
}

void Device::StartIO() {
  if (ioIsRunning_ == UINT64_MAX)
    throw OSException("too many calls to StartIO",
                      kAudioHardwareIllegalOperationError);
  
  if (ioIsRunning_ == 0) {
    OpenConnection();

    ioIsRunning_ = 1;
    numberTimeStamps_ = 0;
//    anchorSampleTime_ = 0;
    anchorHostTime_ = mach_absolute_time();
  } else {
    ++ioIsRunning_;
  }
}

void Device::StopIO() {
  if (ioIsRunning_ == 0)
    throw OSException("IO is not running",
                      kAudioHardwareIllegalOperationError);
  
  --ioIsRunning_;
  if (ioIsRunning_ == 0)
    CloseConnection();
}

void Device::GetZeroTimeStamp(Float64& sampleTime,
                              UInt64& hostTime,
                              UInt64& seed) {
  auto currentHostTime = mach_absolute_time();
  auto hostTicksPerRingBuffer = hostTicksPerFrame_ * ringBufferSize;
  auto hostTickOffset = (numberTimeStamps_ + 1) * hostTicksPerRingBuffer;
  auto nextHostTime = anchorHostTime_ + static_cast<UInt64>(hostTickOffset);
  
  if (nextHostTime <= currentHostTime)
    ++numberTimeStamps_;
  
  sampleTime = numberTimeStamps_ * ringBufferSize;
  hostTime = anchorHostTime_ + numberTimeStamps_ * hostTicksPerRingBuffer;
  seed = 1;
}

std::pair<bool, bool> Device::WillDoIOOperation(UInt32 operationID) const {
  if (operationID == kAudioServerPlugInIOOperationWriteMix)
    return {connectionIsOpen_, true};
  
  return {false, true};
}

void Device::BeginIOOperation(UInt32 operationID,
                              UInt32 ioBufferFrameSize,
                              const AudioServerPlugInIOCycleInfo& ioCycleInfo) {
#pragma unused(operationID, ioBufferFrameSize, ioCycleInfo)
}

void Device::DoIOOperation(AudioObjectID streamObjectID,
                           UInt32 operationID,
                           UInt32 ioBufferFrameSize,
                           const AudioServerPlugInIOCycleInfo& ioCycleInfo,
                           void* ioMainBuffer,
                           void* ioSecondaryBuffer) {
  if (operationID == kAudioServerPlugInIOOperationWriteMix)
    WriteOutputData(ioBufferFrameSize,
                    ioCycleInfo.mOutputTime.mSampleTime,
                    ioMainBuffer);
}

void Device::EndIOOperation(UInt32 operationID,
                            UInt32 ioBufferFrameSize,
                            const AudioServerPlugInIOCycleInfo& ioCycleInfo) {
#pragma unused(operationID, ioBufferFrameSize, ioCycleInfo)
}

void Device::WriteOutputData(UInt32 ioBufferFrameSize,
                             Float64 sampleTime,
                             const void* buffer) {
  LOG(boost::format("WriteOutputData: ioBufferFrameSize=%1%")
      % ioBufferFrameSize);
  asio::write(*outputSocket_, asio::buffer(buffer, ioBufferFrameSize * 8));
}

void Device::OpenConnection() {
  LOG("Opening connection");
  tcp::resolver resolver(ioService_);
  asio::connect(*outputSocket_,
                resolver.resolve({"192.168.1.40", "19999"}));
  LOG("Connection opened");
  connectionIsOpen_ = true;
}

void Device::CloseConnection() {
  LOG("Closing connection");
  outputSocket_->close();
  connectionIsOpen_ = false;
}
