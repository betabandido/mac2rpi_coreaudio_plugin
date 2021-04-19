#include "C_bindings.h"
#include "Device.h"
#include "log.h"
#include "OSException.h"
#include "PlugIn.h"
#include "types.h"

static AudioServerPlugInDriverInterface	gDriverInterface =
{
  nullptr,
  QueryInterface,
  AddRef,
  Release,
  Initialize,
  CreateDevice,
  DestroyDevice,
  AddDeviceClient,
  RemoveDeviceClient,
  PerformDeviceConfigurationChange,
  AbortDeviceConfigurationChange,
  HasProperty,
  IsPropertySettable,
  GetPropertyDataSize,
  GetPropertyData,
  SetPropertyData,
  StartIO,
  StopIO,
  GetZeroTimeStamp,
  WillDoIOOperation,
  BeginIOOperation,
  DoIOOperation,
  EndIOOperation
};

static AudioServerPlugInDriverInterface*
    gDriverInterfacePtr = &gDriverInterface;

static AudioServerPlugInDriverRef
    gDriverInterfaceRef = &gDriverInterfacePtr;

static std::atomic<UInt32> gDriverRefCount{1};

void* CreatePlugIn(CFAllocatorRef allocator,
                   CFUUIDRef requestedTypeUUID) {
#pragma unused(allocator)
  
  try {
    if (CFEqual(requestedTypeUUID, kAudioServerPlugInTypeUUID)) {
      PlugIn::GetInstance();
      LOG("Plug-in was successfully created");
      return gDriverInterfaceRef;
    }
  } catch (...) {
    LOG("error creating the plug-in");
  }
  
  return nullptr;
}

static HRESULT QueryInterface(void* driver,
                               REFIID UUID,
                               LPVOID* interface) {
  try {
    if (driver != gDriverInterfaceRef)
      throw OSException("bad driver reference",
                        kAudioHardwareBadObjectError);

    if (interface == nullptr)
      throw OSException("no place to store the interface",
                        kAudioHardwareIllegalOperationError);
    
    auto requestedUUID = CFUUIDCreateFromUUIDBytes(nullptr, UUID);
    if (requestedUUID == nullptr)
      throw OSException("failed to create CFUUID",
                        kAudioHardwareIllegalOperationError);
    
    if (!CFEqual(requestedUUID, IUnknownUUID)
        && !CFEqual(requestedUUID, kAudioServerPlugInDriverInterfaceUUID))
      throw OSException("requested interface is unsupported", E_NOINTERFACE);
    
    if (gDriverRefCount == UINT32_MAX)
      throw OSException("out of references", E_NOINTERFACE);
    
    ++gDriverRefCount;
    *interface = gDriverInterfaceRef;
    LOG("Returning driver interface");
    return S_OK;
  } catch (const OSException& e) {
    LOG(boost::format("QueryInterface: %1%") % e.what());
    return e.status();
  } catch (...) {
    return kAudioHardwareUnspecifiedError;
  }
}

static ULONG AddRef(void* driver) {
  try {
    if (driver != gDriverInterfaceRef)
      throw OSException("bad driver reference");
  
    if (gDriverRefCount == UINT32_MAX)
      throw OSException("out of references");
  
    ++gDriverRefCount;
    LOG(boost::format("AddRef: #references=%1%") % gDriverRefCount);
    return gDriverRefCount;
  } catch (const OSException& e) {
    LOG(boost::format("AddRef: %1%") % e.what());
  } catch (...) {}

  return 0;
}

static ULONG Release(void* driver) {
  try {
    if (driver != gDriverInterfaceRef)
      throw OSException("bad driver reference");
    
    if (gDriverRefCount == 0)
      throw OSException("too many releases");
    
    --gDriverRefCount;
    LOG(boost::format("Release: #references=%1%") % gDriverRefCount);
    return gDriverRefCount;
  } catch (const OSException& e) {
    LOG(boost::format("Release: %1%") % e.what());
  } catch (...) {}
  
  return 0;
}

static OSStatus Initialize(AudioServerPlugInDriverRef driver,
                           AudioServerPlugInHostRef host) {
  try {
    LOG("Initializing mac2rpi");
    
    if (driver != gDriverInterfaceRef)
      throw OSException("bad driver reference");

    PlugIn::GetInstance().SetHost(host);
    auto& device = static_cast<Device&>(AudioObjectMap::FindObject(kObjectID_Device));
    device.ComputeHostTicksPerFrame();
    return 0;
  } catch (const OSException& e) {
    LOG(boost::format("Release: %1%") % e.what());
    return e.status();
  } catch (...) {
    return kAudioHardwareUnspecifiedError;
  }
}

static OSStatus	CreateDevice(AudioServerPlugInDriverRef driver,
                             CFDictionaryRef description,
                             const AudioServerPlugInClientInfo* clientInfo,
                             AudioObjectID* deviceObjectID) {
#pragma unused(driver, description, clientInfo, deviceObjectID)
  
  return kAudioHardwareUnsupportedOperationError;
}

static OSStatus	DestroyDevice(AudioServerPlugInDriverRef driver,
                              AudioObjectID deviceObjectID) {
#pragma unused(driver, deviceObjectID)

  return kAudioHardwareUnsupportedOperationError;
}

static OSStatus	AddDeviceClient(AudioServerPlugInDriverRef driver,
                                AudioObjectID deviceObjectID,
                                const AudioServerPlugInClientInfo* clientInfo) {
#pragma unused(driver, deviceObjectID, clientInfo)

  return 0;
}

static OSStatus	RemoveDeviceClient(AudioServerPlugInDriverRef driver,
                                   AudioObjectID deviceObjectID,
                                   const AudioServerPlugInClientInfo* clientInfo) {
#pragma unused(driver, deviceObjectID, clientInfo)

  return 0;
}

static OSStatus
PerformDeviceConfigurationChange(AudioServerPlugInDriverRef driver,
                                 AudioObjectID deviceObjectID,
                                 UInt64 changeAction,
                                 void* change_info) {
  LOG("####### PerformDeviceConfigurationChange");
  // TODO
  return 0;
}

static OSStatus
AbortDeviceConfigurationChange(AudioServerPlugInDriverRef driver,
                               AudioObjectID deviceObjectID,
                               UInt64 changeAction,
                               void* change_info) {
  LOG("####### AbortDeviceConfigurationChange");
  // TODO
  return 0;
}

#pragma mark Property Operations

static Boolean HasProperty(AudioServerPlugInDriverRef driver,
                           AudioObjectID objectID,
                           pid_t clientProcessID,
                           const AudioObjectPropertyAddress* address) {
  try {
    if (driver != gDriverInterfaceRef)
      throw OSException("bad driver reference");
    if (address == nullptr)
      throw OSException("no address");
        
    auto& object = AudioObjectMap::FindObject(objectID);
    return object.HasProperty(clientProcessID, *address);
  } catch (const OSException& e) {
    LOG(boost::format("HasProperty: %s") % e.what());
  } catch (...) {}
  
  return false;
}

static OSStatus	IsPropertySettable(AudioServerPlugInDriverRef driver,
                                   AudioObjectID objectID,
                                   pid_t clientProcessID,
                                   const AudioObjectPropertyAddress* address,
                                   Boolean* isSettable) {
  try {
    if (driver != gDriverInterfaceRef)
      throw OSException("bad driver reference",
                        kAudioHardwareBadObjectError);
    if (address == nullptr)
      throw OSException("no address",
                        kAudioHardwareIllegalOperationError);
    if (isSettable == nullptr)
      throw OSException("no place to put the return value",
                        kAudioHardwareIllegalOperationError);
    
    auto& object = AudioObjectMap::FindObject(objectID);
    *isSettable = object.IsPropertySettable(clientProcessID, *address);
    
    return 0;
  } catch (const OSException& e) {
    LOG(boost::format("IsPropertySettable: %s") % e.what());
    return e.status();
  } catch (...) {
    return kAudioHardwareUnspecifiedError;
  }
}

static OSStatus	GetPropertyDataSize(AudioServerPlugInDriverRef driver,
                                    AudioObjectID objectID,
                                    pid_t clientProcessID,
                                    const AudioObjectPropertyAddress* address,
                                    UInt32 qualifierDataSize,
                                    const void* qualifierData,
                                    UInt32* dataSize) {
  try {
    if (driver != gDriverInterfaceRef)
      throw OSException("bad driver reference",
                        kAudioHardwareBadObjectError);
    if (address == nullptr)
      throw OSException("no address",
                        kAudioHardwareIllegalOperationError);
    if (dataSize == nullptr)
      throw OSException("no place to put the return value",
                        kAudioHardwareIllegalOperationError);
    
    auto& object = AudioObjectMap::FindObject(objectID);
    *dataSize = object.GetPropertyDataSize(clientProcessID,
                                           *address,
                                           qualifierDataSize,
                                           qualifierData);
    return 0;
  } catch (const OSException& e) {
    LOG(boost::format("GetPropertyDataSize: %s") % e.what());
    return e.status();
  } catch (...) {
    return kAudioHardwareUnspecifiedError;
  }
}

static OSStatus	GetPropertyData(AudioServerPlugInDriverRef driver,
                                AudioObjectID objectID,
                                pid_t clientProcessID,
                                const AudioObjectPropertyAddress* address,
                                UInt32 qualifierDataSize,
                                const void* qualifierData,
                                UInt32 inDataSize,
                                UInt32* outDataSize,
                                void* outData) {
  try {
    if (driver != gDriverInterfaceRef)
      throw OSException("bad driver reference",
                        kAudioHardwareBadObjectError);
    if (address == nullptr)
      throw OSException("no address",
                        kAudioHardwareIllegalOperationError);
    if (outDataSize == nullptr || outData == nullptr)
      throw OSException("no place to put the return value",
                        kAudioHardwareIllegalOperationError);
    
//    LOG(boost::format("GetPropertyData: objectID=%1%, selector=%2%")
//        % objectID
//        % address->mSelector);
    
    auto& object = AudioObjectMap::FindObject(objectID);
    *outDataSize = object.GetPropertyData(clientProcessID,
                                          *address,
                                          qualifierDataSize,
                                          qualifierData,
                                          inDataSize,
                                          outData);
    return 0;
  } catch (const OSException& e) {
    LOG(boost::format("GetPropertyData: %s") % e.what());
    return e.status();
  } catch (...) {
    return kAudioHardwareUnspecifiedError;
  }
}

static OSStatus	SetPropertyData(AudioServerPlugInDriverRef driver,
                                AudioObjectID objectID,
                                pid_t clientProcessID,
                                const AudioObjectPropertyAddress* address,
                                UInt32 qualifierDataSize,
                                const void* qualifierData,
                                UInt32 dataSize,
                                const void* data) {
  try {
    if (driver != gDriverInterfaceRef)
      throw OSException("bad driver reference",
                        kAudioHardwareBadObjectError);
    if (address == nullptr)
      throw OSException("no address",
                        kAudioHardwareIllegalOperationError);
    
    LOG(boost::format(">>>> SetPropertyData: objectID=%1%, selector=%2%")
        % objectID
        % address->mSelector);
    
    auto& object = AudioObjectMap::FindObject(objectID);
    auto result = object.SetPropertyData(clientProcessID,
                                         *address,
                                         qualifierDataSize,
                                         qualifierData,
                                         dataSize,
                                         data);
    
    // TODO
#if 0
    if (result.first > 0) {
      gPlugInHost->PropertiesChanged(gPlugInHost,
                                     objectID,
                                     result.first,
                                     result.second.data());
    }
#endif
    
    return 0;
  } catch (const OSException& e) {
    LOG(boost::format("SetPropertyData: %s") % e.what());
    return e.status();
  } catch (...) {
    return kAudioHardwareUnspecifiedError;
  }
}

static OSStatus	StartIO(AudioServerPlugInDriverRef driver,
                        AudioObjectID deviceObjectID,
                        UInt32 clientID) {
#pragma unused(clientID)

  try {
    if (driver != gDriverInterfaceRef)
      throw OSException("bad driver reference",
                        kAudioHardwareBadObjectError);
    
    LOG("*** StartIO ***");
  
    auto& device = static_cast<Device&>(AudioObjectMap::FindObject(deviceObjectID));
    device.StartIO();
    return 0;
  } catch (const OSException& e) {
    LOG(boost::format("StartIO: %s") % e.what());
    return e.status();
  } catch (...) {
    LOG(boost::format("StartIO: unspecified error"));
    return kAudioHardwareUnspecifiedError;
  }
}

static OSStatus	StopIO(AudioServerPlugInDriverRef driver,
                       AudioObjectID deviceObjectID,
                       UInt32 clientID) {
#pragma unused(clientID)

  try {
    if (driver != gDriverInterfaceRef)
      throw OSException("bad driver reference",
                        kAudioHardwareBadObjectError);
    
    LOG("*** StopIO ***");
    
    auto& device = static_cast<Device&>(AudioObjectMap::FindObject(deviceObjectID));
    device.StopIO();
    return 0;
  } catch (const OSException& e) {
    LOG(boost::format("StopIO: %s") % e.what());
    return e.status();
  } catch (...) {
    LOG(boost::format("StopIO: unspecified error"));
    return kAudioHardwareUnspecifiedError;
  }
}

static OSStatus	GetZeroTimeStamp(AudioServerPlugInDriverRef driver,
                                 AudioObjectID deviceObjectID,
                                 UInt32 clientID,
                                 Float64* sampleTime,
                                 UInt64* hostTime,
                                 UInt64* seed) {
  try {
    if (driver != gDriverInterfaceRef)
      throw OSException("bad driver reference",
                        kAudioHardwareBadObjectError);
    
    auto& device = static_cast<Device&>(AudioObjectMap::FindObject(deviceObjectID));
    device.GetZeroTimeStamp(*sampleTime, *hostTime, *seed);
    return 0;
  } catch (const OSException& e) {
    LOG(boost::format("GetZeroTimeStamp: %s") % e.what());
    return e.status();
  } catch (...) {
    return kAudioHardwareUnspecifiedError;
  }
}

static OSStatus	WillDoIOOperation(AudioServerPlugInDriverRef driver,
                                  AudioObjectID deviceObjectID,
                                  UInt32 clientID,
                                  UInt32 operationID,
                                  Boolean* willDo,
                                  Boolean* willDoInPlace) {
#pragma unused(clientID)
  
  try {
    if (driver != gDriverInterfaceRef)
      throw OSException("bad driver reference",
                        kAudioHardwareBadObjectError);

    if (willDo == nullptr)
      throw OSException("no place to put the will-do value",
                        kAudioHardwareIllegalOperationError);

    if (willDoInPlace == nullptr)
      throw OSException("no place to put the will-do in-place value",
                        kAudioHardwareIllegalOperationError);

//    LOG(boost::format("WillDoIOOperation: deviceObjectID=%1% operationID=%2%")
//        % deviceObjectID
//        % operationID);

    auto& device = static_cast<Device&>(AudioObjectMap::FindObject(deviceObjectID));
    std::tie(*willDo, *willDoInPlace) = device.WillDoIOOperation(operationID);
    return 0;
  } catch (const OSException& e) {
    LOG(boost::format("WillDoIOOperation: %s") % e.what());
    return e.status();
  } catch (...) {
    return kAudioHardwareUnspecifiedError;
  }
}

static OSStatus	BeginIOOperation(AudioServerPlugInDriverRef driver,
                                 AudioObjectID deviceObjectID,
                                 UInt32 clientID,
                                 UInt32 operationID,
                                 UInt32 ioBufferFrameSize,
                                 const AudioServerPlugInIOCycleInfo* ioCycleInfo) {
#pragma unused(clientID)

  try {
    if (driver != gDriverInterfaceRef)
      throw OSException("bad driver reference",
                        kAudioHardwareBadObjectError);
    
    if (ioCycleInfo == nullptr)
      throw OSException("no cycle info",
                        kAudioHardwareIllegalOperationError);
    
//    LOG(boost::format("BeginIOOperation: deviceObjectID=%1% operationID=%2%")
//        % deviceObjectID
//        % operationID);
    
    auto& device = static_cast<Device&>(AudioObjectMap::FindObject(deviceObjectID));
    device.BeginIOOperation(operationID,
                            ioBufferFrameSize,
                            *ioCycleInfo);
    return 0;
  } catch (const OSException& e) {
    LOG(boost::format("BeginIOOperation: %s") % e.what());
    return e.status();
  } catch (...) {
    return kAudioHardwareUnspecifiedError;
  }
}

static OSStatus	DoIOOperation(AudioServerPlugInDriverRef driver,
                              AudioObjectID deviceObjectID,
                              AudioObjectID streamObjectID,
                              UInt32 clientID,
                              UInt32 operationID,
                              UInt32 ioBufferFrameSize,
                              const AudioServerPlugInIOCycleInfo* ioCycleInfo,
                              void* ioMainBuffer,
                              void* ioSecondaryBuffer) {
#pragma unused(clientID)

  try {
    if (driver != gDriverInterfaceRef)
      throw OSException("bad driver reference",
                        kAudioHardwareBadObjectError);
    
    if (ioCycleInfo == nullptr)
      throw OSException("no cycle info",
                        kAudioHardwareIllegalOperationError);
    
    LOG(boost::format("DoIOOperation: deviceObjectID=%1% operationID=%2%")
        % deviceObjectID
        % operationID);

    auto& device = static_cast<Device&>(AudioObjectMap::FindObject(deviceObjectID));
    device.DoIOOperation(streamObjectID,
                         operationID,
                         ioBufferFrameSize,
                         *ioCycleInfo,
                         ioMainBuffer,
                         ioSecondaryBuffer);
    return 0;
  } catch (const OSException& e) {
    LOG(boost::format("DoIOOperation: %s") % e.what());
    return e.status();
  } catch (...) {
    return kAudioHardwareUnspecifiedError;
  }
}

static OSStatus	EndIOOperation(AudioServerPlugInDriverRef driver,
                               AudioObjectID deviceObjectID,
                               UInt32 clientID,
                               UInt32 operationID,
                               UInt32 ioBufferFrameSize,
                               const AudioServerPlugInIOCycleInfo* ioCycleInfo) {
#pragma unused(clientID)

  try {
    if (driver != gDriverInterfaceRef)
      throw OSException("bad driver reference",
                        kAudioHardwareBadObjectError);
    
    if (ioCycleInfo == nullptr)
      throw OSException("no cycle info",
                        kAudioHardwareIllegalOperationError);
    
//    LOG(boost::format("EndIOOperation: deviceObjectID=%1% operationID=%2%")
//        % deviceObjectID
//        % operationID);

    auto& device = static_cast<Device&>(AudioObjectMap::FindObject(deviceObjectID));
    device.EndIOOperation(operationID,
                          ioBufferFrameSize,
                          *ioCycleInfo);
    return 0;
  } catch (const OSException& e) {
    LOG(boost::format("EndIOOperation: %s") % e.what());
    return e.status();
  } catch (...) {
    return kAudioHardwareUnspecifiedError;
  }
}
