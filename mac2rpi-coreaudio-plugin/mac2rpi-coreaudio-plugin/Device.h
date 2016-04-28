#ifndef Device_h
#define Device_h

#include <atomic>

#include <boost/asio.hpp>

#include "AudioObject.h"

class Stream;
class MuteControl;
class VolumeControl;

class Device : public AudioObject {
public:
  static constexpr std::array<Float64, 2> availableSampleRates = {{
    44100.0, 44800.0
  }};
  
  static constexpr Float32 volumeMinDB = -96.0;
  static constexpr Float32 volumeMaxDB = 6.0;
  
  Device();
  
  virtual ~Device() {}
  
  Boolean HasProperty(pid_t clientProcessID,
                      const AudioObjectPropertyAddress& address) const override;
  
  Boolean IsPropertySettable(pid_t clientProcessID,
                             const AudioObjectPropertyAddress& address) const override;
  
  UInt32 GetPropertyDataSize(pid_t clientProcessID,
                             const AudioObjectPropertyAddress& address,
                             UInt32 qualifierDataSize,
                             const void* qualifierData) const override;
  
  UInt32 GetPropertyData(pid_t clientProcessID,
                         const AudioObjectPropertyAddress& address,
                         UInt32 qualifierDataSize,
                         const void* qualifierData,
                         UInt32 dataSize,
                         void* data) const override;
  
  std::pair<UInt32, ChangedPropertyList>
  SetPropertyData(pid_t clientProcessID,
                  const AudioObjectPropertyAddress& address,
                  UInt32 qualifierDataSize,
                  const void* qualifierData,
                  UInt32 dataSize,
                  const void* data) override;
  
  void ComputeHostTicksPerFrame();
  
  /**
   *
   */
  void StartIO();

  /**
   *
   */
  void StopIO();

  /**
   *
   */
  void GetZeroTimeStamp(Float64& sampleTime,
                        UInt64& hostTime,
                        UInt64& seed);
  
  /**
   *
   */
  std::pair<bool, bool> WillDoIOOperation(UInt32 operationID) const;
  
  /**
   *
   */
  void BeginIOOperation(UInt32 operationID,
                        UInt32 ioBufferFrameSize,
                        const AudioServerPlugInIOCycleInfo& ioCycleInfo);
  
  /**
   *
   */
  void DoIOOperation(AudioObjectID streamObjectID,
                     UInt32 operationID,
                     UInt32 ioBufferFrameSize,
                     const AudioServerPlugInIOCycleInfo& ioCycleInfo,
                     void* ioMainBuffer,
                     void* ioSecondaryBuffer);

  /**
   *
   */
  void EndIOOperation(UInt32 operationID,
                      UInt32 ioBufferFrameSize,
                      const AudioServerPlugInIOCycleInfo& ioCycleInfo);

  /**
   *
   */
  void WriteOutputData(UInt32 ioBufferFrameSize,
                       Float64 sampleTime,
                       const void* buffer);

  /**
   *
   */
  Float64 SampleRate() const { return sampleRate_; }
  
  /**
   *
   */
  Float32 OutputVolume() const { return outputVolume_; }
  
  /**
   *
   */
  bool OutputMute() const { return outputMute_; }
  
private:
  typedef boost::asio::ip::tcp::socket TCPSocket;

  /** 1 stream (output stream). */
  static constexpr unsigned numberOfStreams { 1 };
  
  /** 2 controls (volume and mute). */
  static constexpr unsigned numberOfControls { 2 };
  
  /** Total number of sub-objects. */
  static constexpr unsigned numberOfSubObjects
      { numberOfStreams + numberOfControls };
  
  /** Number of channels (left + right). */
  static constexpr unsigned numberOfChannels { 2 };
  
  /** Size of the imaginary buffer size. */
  static constexpr unsigned ringBufferSize { 4096 };
    
  std::atomic<Float64> sampleRate_ { 44100.0 };
  std::atomic<Float32> outputVolume_ { 0 };
  std::atomic<bool> outputMute_ { false };
  
//  mutable std::mutex stateMutex_;
  
  std::atomic<UInt64> ioIsRunning_ { 0 };
  std::atomic<Float64> hostTicksPerFrame_ { 0 };
  UInt64 numberTimeStamps_ { 0 };
//  Float64 anchorSampleTime_ { 0 };
  UInt64 anchorHostTime_ { 0 };
  
  std::shared_ptr<Stream> outputStream_;
  std::shared_ptr<VolumeControl> volumeControl_;
  std::shared_ptr<MuteControl> muteControl_;
  
  boost::asio::io_service ioService_;
  std::unique_ptr<TCPSocket> outputSocket_;
  
  void OpenConnection();
  void CloseConnection();
};

#endif /* Device_h */
