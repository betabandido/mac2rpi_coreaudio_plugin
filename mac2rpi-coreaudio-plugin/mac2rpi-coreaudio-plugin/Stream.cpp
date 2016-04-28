#include "Stream.h"

#include "Device.h"
#include "log.h"
#include "OSException.h"

Stream::Stream(AudioObjectID objectID, Device& device)
  : AudioObject(objectID,
                kAudioStreamClassID,
                kAudioObjectClassID,
                device.ObjectID())
  , device_(device)
{}

Boolean Stream::HasProperty(pid_t clientProcessID,
                            const AudioObjectPropertyAddress& address) const {

  LOG(boost::format("StreamHasProperty: selector=%1%")
      % StreamPropertyToString(address.mSelector));

  switch (address.mSelector) {
    case kAudioStreamPropertyIsActive:
    case kAudioStreamPropertyDirection:
    case kAudioStreamPropertyTerminalType:
    case kAudioStreamPropertyStartingChannel:
    case kAudioStreamPropertyLatency:
    case kAudioStreamPropertyVirtualFormat:
    case kAudioStreamPropertyPhysicalFormat:
    case kAudioStreamPropertyAvailableVirtualFormats:
    case kAudioStreamPropertyAvailablePhysicalFormats:
      return true;
  }
  
  return AudioObject::HasProperty(clientProcessID, address);
}

Boolean Stream::IsPropertySettable(pid_t clientProcessID,
                                   const AudioObjectPropertyAddress& address) const {
  switch (address.mSelector) {
    case kAudioStreamPropertyDirection:
    case kAudioStreamPropertyTerminalType:
    case kAudioStreamPropertyStartingChannel:
    case kAudioStreamPropertyLatency:
    case kAudioStreamPropertyAvailableVirtualFormats:
    case kAudioStreamPropertyAvailablePhysicalFormats:
      return false;
      
    case kAudioStreamPropertyIsActive:
    case kAudioStreamPropertyVirtualFormat:
    case kAudioStreamPropertyPhysicalFormat:
      return true;
  };
  
  return AudioObject::IsPropertySettable(clientProcessID, address);
}

UInt32 Stream::GetPropertyDataSize(pid_t clientProcessID,
                                   const AudioObjectPropertyAddress& address,
                                   UInt32 qualifierDataSize,
                                   const void* qualifierData) const {
  switch (address.mSelector) {
    case kAudioStreamPropertyIsActive:
    case kAudioStreamPropertyDirection:
    case kAudioStreamPropertyTerminalType:
    case kAudioStreamPropertyStartingChannel:
    case kAudioStreamPropertyLatency:
      return sizeof(UInt32);

    case kAudioStreamPropertyVirtualFormat:
    case kAudioStreamPropertyPhysicalFormat:
      return sizeof(AudioStreamBasicDescription);

    case kAudioStreamPropertyAvailableVirtualFormats:
    case kAudioStreamPropertyAvailablePhysicalFormats:
      return Device::availableSampleRates.size()
          * sizeof(AudioStreamBasicDescription);
  };
  
  return AudioObject::GetPropertyDataSize(clientProcessID,
                                          address,
                                          qualifierDataSize,
                                          qualifierData);
}

UInt32 Stream::GetPropertyData(pid_t clientProcessID,
                               const AudioObjectPropertyAddress& address,
                               UInt32 qualifierDataSize,
                               const void* qualifierData,
                               UInt32 dataSize,
                               void* data) const {
  
  LOG(boost::format("StreamGetPropertyData: selector=%1%")
      % StreamPropertyToString(address.mSelector));

  switch (address.mSelector) {
    case kAudioStreamPropertyIsActive:
    {
      LOG("########## Get IsActive: UNSUPPORTED");

      // TODO
      return 0;
//      std::lock_guard<std::mutex> lock(g_pluginStateMutex);
//      return GetPropertyDataImpl<UInt32>
//          (dataSize, gStream_Output_IsActive, data);
    }
      
    case kAudioStreamPropertyDirection:
      // 0: output stream; 1: input stream
      return GetPropertyDataImpl<UInt32>
          (dataSize, 0, data);
      
    case kAudioStreamPropertyTerminalType:
      return GetPropertyDataImpl<UInt32>
          (dataSize, kAudioStreamTerminalTypeSpeaker, data);

    case kAudioStreamPropertyStartingChannel:
      return GetPropertyDataImpl<UInt32>
          (dataSize, 1, data);

    case kAudioStreamPropertyLatency:
      return GetPropertyDataImpl<UInt32>
          (dataSize, 0, data);
      
    case kAudioStreamPropertyVirtualFormat:
    case kAudioStreamPropertyPhysicalFormat:
    {
      CheckOutDataSize(dataSize, sizeof(AudioStreamBasicDescription));
      auto& desc = *static_cast<AudioStreamBasicDescription*>(data);
      desc.mSampleRate = device_.SampleRate();
      desc.mFormatID = kAudioFormatLinearPCM;
      desc.mFormatFlags =
          kAudioFormatFlagIsFloat
          | kAudioFormatFlagsNativeEndian
          | kAudioFormatFlagIsPacked;
      desc.mBytesPerPacket = 8;
      desc.mFramesPerPacket = 1;
      desc.mBytesPerFrame = 8;
      desc.mChannelsPerFrame = 2;
      desc.mBitsPerChannel = 32;
      
      return sizeof(AudioStreamBasicDescription);
    }
      
    case kAudioStreamPropertyAvailableVirtualFormats:
    case kAudioStreamPropertyAvailablePhysicalFormats:
    {
      UInt32 itemCount = dataSize / sizeof(AudioStreamRangedDescription);
      itemCount = std::min<UInt32>(itemCount, Device::availableSampleRates.size());
      
      for (unsigned i = 0; i < itemCount; i++) {
        auto& desc = static_cast<AudioStreamRangedDescription*>(data)[i];
        auto sample_rate = Device::availableSampleRates[i];
        desc.mFormat.mSampleRate = sample_rate;
        desc.mFormat.mFormatID = kAudioFormatLinearPCM;
        desc.mFormat.mFormatFlags =
            kAudioFormatFlagIsFloat
            | kAudioFormatFlagsNativeEndian
            | kAudioFormatFlagIsPacked;
        desc.mFormat.mBytesPerPacket = 8;
        desc.mFormat.mFramesPerPacket = 1;
        desc.mFormat.mBytesPerFrame = 8;
        desc.mFormat.mChannelsPerFrame = 2;
        desc.mFormat.mBitsPerChannel = 32;
        desc.mSampleRateRange.mMinimum = sample_rate;
        desc.mSampleRateRange.mMaximum = sample_rate;
      }

      return itemCount * sizeof(AudioStreamRangedDescription);
    }
  };
  
  return AudioObject::GetPropertyData(clientProcessID,
                                      address,
                                      qualifierDataSize,
                                      qualifierData,
                                      dataSize,
                                      data);
}

std::pair<UInt32, Stream::ChangedPropertyList>
Stream::SetPropertyData(pid_t clientProcessID,
                        const AudioObjectPropertyAddress& address,
                        UInt32 qualifierDataSize,
                        const void* qualifierData,
                        UInt32 dataSize,
                        const void* data) {
  switch (address.mSelector) {
    case kAudioStreamPropertyIsActive:
      LOG("########## Set IsActive: UNSUPPORTED");
      // TODO
      break;
#if 0
    {
      check_in_data_size(dataSize, sizeof(UInt32));
      std::lock_guard<std::mutex> lock(gPlugInStateMutex);
      bool isActive = *(static_cast<UInt32*>(data)) != 0;
      if (gStream_Output_IsActive != isActive) {
        gStream_Output_IsActive = isActive;
        ChangedAddressList changedAddresses;
        changedAddresses[0].mSelector = kAudioStreamPropertyIsActive;
        changedAddresses[0].mScope = kAudioObjectPropertyScopeGlobal;
        changedAddresses[0].mElement = kAudioObjectPropertyElementMaster;
        return std::make_pair(1, changedAddresses);
      }
    }
#endif
      
    case kAudioStreamPropertyVirtualFormat:
    case kAudioStreamPropertyPhysicalFormat:
      // Changing the stream format needs to be handled via the
      // RequestConfigChange/PerformConfigChange machinery.
      // Note that because this device only supports 2 channel 32 bit float
      // data, the only thing that can change is the sample rate.
      LOG("########## Set Stream format: UNSUPPORTED");

      // TODO
    {
      CheckInDataSize(dataSize, sizeof(AudioStreamBasicDescription));
      auto desc = static_cast<const AudioStreamBasicDescription*>(data);
      
      if (desc->mFormatID != kAudioFormatLinearPCM
          || desc->mFormatFlags != (kAudioFormatFlagIsFloat
                                    | kAudioFormatFlagsNativeEndian
                                    | kAudioFormatFlagIsPacked)
          || desc->mBytesPerPacket != 8
          || desc->mFramesPerPacket != 1
          || desc->mBytesPerFrame != 8
          || desc->mChannelsPerFrame != 2
          || desc->mBitsPerChannel != 32
          || ((desc->mSampleRate != 44100.0) && (desc->mSampleRate != 48000.0)))
        throw OSException("unsupported stream format",
                          kAudioDeviceUnsupportedFormatError);
      
//      // XXX can't we just use atomics?
//      Float64 oldSampleRate;
//      {
//        std::lock_guard<std::mutex> lock(gPlugInStateMutex);
//        oldSampleRate = gDeviceSampleRate;
//      }
//      
//      if (desc->sampleRate != oldSampleRate) {
//        LOG(boost::format("Changing sample rate: %1%") % desc->sampleRate);
//        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
//          ^{ gPlugInHost->RequestDeviceConfigurationChange(gPluginHost,
//                                                           kObjectID_Device,
//                                                           static_cast<UInt64>(sampleRate),
//                                                           nullptr);
//          });
//      }
      
//      return std::make_pair(0, ChangedAddressList{});
      break;
    }
  };
  
  return AudioObject::SetPropertyData(clientProcessID,
                                      address,
                                      qualifierDataSize,
                                      qualifierData,
                                      dataSize,
                                      data);
}
