#ifndef C_bindings_h
#define C_bindings_h

#include <CoreAudio/AudioServerPlugIn.h>

extern "C" void* CreatePlugIn(CFAllocatorRef allocator,
                              CFUUIDRef requestedTypeUUID);

static HRESULT QueryInterface(void* driver,
                              REFIID UUID,
                              LPVOID* interface);

static ULONG AddRef(void* driver);

static ULONG Release(void* driver);

static OSStatus Initialize(AudioServerPlugInDriverRef driver,
                           AudioServerPlugInHostRef host);

static OSStatus	CreateDevice(AudioServerPlugInDriverRef driver,
                             CFDictionaryRef description,
                             const AudioServerPlugInClientInfo* clientInfo,
                             AudioObjectID* deviceObjectID);

static OSStatus	DestroyDevice(AudioServerPlugInDriverRef driver,
                              AudioObjectID deviceObjectID);

static OSStatus	AddDeviceClient(AudioServerPlugInDriverRef driver,
                                AudioObjectID deviceObjectID,
                                const AudioServerPlugInClientInfo* clientInfo);

static OSStatus	RemoveDeviceClient(AudioServerPlugInDriverRef driver,
                                   AudioObjectID deviceObjectID,
                                   const AudioServerPlugInClientInfo* clientInfo);

static OSStatus
PerformDeviceConfigurationChange(AudioServerPlugInDriverRef driver,
                                 AudioObjectID deviceObjectID,
                                 UInt64 changeAction,
                                 void* change_info);

static OSStatus
AbortDeviceConfigurationChange(AudioServerPlugInDriverRef driver,
                               AudioObjectID deviceObjectID,
                               UInt64 changeAction,
                               void* change_info);

static Boolean HasProperty(AudioServerPlugInDriverRef driver,
                           AudioObjectID objectID,
                           pid_t clientProcessID,
                           const AudioObjectPropertyAddress* address);


static OSStatus	IsPropertySettable(AudioServerPlugInDriverRef driver,
                                   AudioObjectID objectID,
                                   pid_t clientProcessID,
                                   const AudioObjectPropertyAddress* address,
                                   Boolean* isSettable);

static OSStatus	GetPropertyDataSize(AudioServerPlugInDriverRef driver,
                                    AudioObjectID objectID,
                                    pid_t clientProcessID,
                                    const AudioObjectPropertyAddress* address,
                                    UInt32 qualifierDataSize,
                                    const void* qualifierData,
                                    UInt32* dataSize);

static OSStatus	GetPropertyData(AudioServerPlugInDriverRef driver,
                                AudioObjectID objectID,
                                pid_t clientProcessID,
                                const AudioObjectPropertyAddress* address,
                                UInt32 qualifierDataSize,
                                const void* qualifierData,
                                UInt32 inDataSize,
                                UInt32* outDataSize,
                                void* outData);

static OSStatus	SetPropertyData(AudioServerPlugInDriverRef driver,
                                AudioObjectID objectID,
                                pid_t clientProcessID,
                                const AudioObjectPropertyAddress* address,
                                UInt32 qualifierDataSize,
                                const void* qualifierData,
                                UInt32 dataSize,
                                const void* data);

static OSStatus	StartIO(AudioServerPlugInDriverRef driver,
                        AudioObjectID deviceObjectID,
                        UInt32 clientID);

static OSStatus	StopIO(AudioServerPlugInDriverRef driver,
                       AudioObjectID deviceObjectID,
                       UInt32 clientID);

static OSStatus	GetZeroTimeStamp(AudioServerPlugInDriverRef driver,
                                 AudioObjectID deviceObjectID,
                                 UInt32 clientID,
                                 Float64* sampleTime,
                                 UInt64* hostTime,
                                 UInt64* seed);

static OSStatus	WillDoIOOperation(AudioServerPlugInDriverRef driver,
                                  AudioObjectID deviceObjectID,
                                  UInt32 clientID,
                                  UInt32 operationID,
                                  Boolean* willDo,
                                  Boolean* willDoInPlace);

static OSStatus	BeginIOOperation(AudioServerPlugInDriverRef driver,
                                 AudioObjectID deviceObjectID,
                                 UInt32 clientID,
                                 UInt32 operationID,
                                 UInt32 ioBufferFrameSize,
                                 const AudioServerPlugInIOCycleInfo* ioCycleInfo);

static OSStatus	DoIOOperation(AudioServerPlugInDriverRef driver,
                              AudioObjectID deviceObjectID,
                              AudioObjectID streamObjectID,
                              UInt32 clientID,
                              UInt32 operationID,
                              UInt32 ioBufferFrameSize,
                              const AudioServerPlugInIOCycleInfo* ioCycleInfo,
                              void* ioMainBuffer,
                              void* ioSecondaryBuffer);

static OSStatus	EndIOOperation(AudioServerPlugInDriverRef driver,
                               AudioObjectID deviceObjectID,
                               UInt32 clientID,
                               UInt32 operationID,
                               UInt32 ioBufferFrameSize,
                               const AudioServerPlugInIOCycleInfo* ioCycleInfo);

#endif /* C_bindings_h */
