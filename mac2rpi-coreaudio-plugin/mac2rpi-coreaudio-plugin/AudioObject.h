#ifndef AudioObject_h
#define AudioObject_h

#include <array>
#include <map>
#include <memory>
#include <utility>

#include <CoreAudio/AudioServerPlugIn.h>

/** Base class for all the components in the plug-in. */
class AudioObject {
public:
  /** List of properties changed. Multiple properties can be changed with just
   * a single call to SetPropertyData(). For instance, changing the scalar value
   * of the volume control changes its decibel value as well.
   *
   * XXX In our case we never modify more than two properties at once, so we
   * statically set the size of the array accordingly.
   */
  typedef std::array<AudioObjectPropertyAddress, 2> ChangedPropertyList;
  
  AudioObject(AudioObjectID objectID,
              AudioClassID classID,
              AudioClassID baseClassID,
              AudioObjectID ownerID);
  
  virtual ~AudioObject() {}
  
  AudioObjectID ObjectID() const { return objectID_; }
  
  /** Returns whether the object has a certain property.
   *
   * @param clientProcessID The process ID of the client making the request.
   * @param address Structure containing the property details.
   * @return True if the property is supported; false otherwise.
   */
  virtual Boolean HasProperty(pid_t clientProcessID,
                              const AudioObjectPropertyAddress& address) const;

  /** Returns whether a property can be set (modified).
   *
   * @param clientProcessID The process ID of the client making the request.
   * @param address Structure containing the property details.
   * @return True if the property can be set; false otherwise.
   */
  virtual Boolean IsPropertySettable(pid_t clientProcessID,
                                     const AudioObjectPropertyAddress& address) const;

  /** Returns the necessary storage to store the value of a property.
   *
   * @param clientProcessID The process ID of the client making the request.
   * @param address Structure containing the property details.
   * @param qualifierDataSize Size of qualifier input data.
   * @param qualifierData Qualifier input data.
   * @return The number of bytes needed to store the property.
   */
  virtual UInt32 GetPropertyDataSize(pid_t clientProcessID,
                                     const AudioObjectPropertyAddress& address,
                                     UInt32 qualifierDataSize,
                                     const void* qualifierData) const;
  
  /** Returns the value of a property.
   *
   * @param clientProcessID The process ID of the client making the request.
   * @param address Structure containing the property details.
   * @param qualifierDataSize Size of qualifier input data.
   * @param qualifierData Qualifier input data.
   * @param dataSize Available storage in \p data.
   * @param data Storage area to store the value of the property.
   * @return The number of bytes needed to store the property.
   */
  virtual UInt32 GetPropertyData(pid_t clientProcessID,
                                 const AudioObjectPropertyAddress& address,
                                 UInt32 qualifierDataSize,
                                 const void* qualifierData,
                                 UInt32 dataSize,
                                 void* data) const;

  /** Sets the value of a property.
   *
   * @param clientProcessID The process ID of the client making the request.
   * @param address Structure containing the property details.
   * @param qualifierDataSize Size of qualifier input data.
   * @param qualifierData Qualifier input data.
   * @param dataSize Size of value in \p data.
   * @param data The new value of the property.
   * @return A pair consisting of 1) the number of properties changed, and 2)
   *         an array with the changed properties.
   */
  virtual std::pair<UInt32, ChangedPropertyList>
  SetPropertyData(pid_t clientProcessID,
                  const AudioObjectPropertyAddress& address,
                  UInt32 qualifierDataSize,
                  const void* qualifierData,
                  UInt32 dataSize,
                  const void* data);

private:
  AudioObjectID objectID_;
  AudioClassID classID_;
  AudioClassID baseClassID_;
  AudioObjectID ownerID_;
  
  AudioObject(const AudioObject&) = delete;
  AudioObject& operator=(const AudioObject&) = delete;
};

/** Checks whether the provided storage area is sufficient to hold the value
 * of a property. If it is not sufficient, an exception is thrown.
 *
 * @param provided Provided storage.
 * @param required Required storage.
 */
void CheckOutDataSize(UInt32 provided, UInt32 required);

/** Checks whether the provided data matches the size of a property. If that is
 not the case, an exception is thrown.
 
 @param provided Provided data size.
 @param required Required data size.
 */
void CheckInDataSize(UInt32 provided, UInt32 required);

/** Helper function to return a property and its size.
 *
 * @param dataSize Size of the provided storage area.
 * @param value Value of the property.
 * @param data Storage area where to put the value of the property.
 * @return Number of bytes used to store the property.
 */
template<typename T>
UInt32 GetPropertyDataImpl(UInt32 dataSize, T value, void* data) {
  CheckOutDataSize(dataSize, sizeof(T));
  *(static_cast<T*>(data)) = value;
  return sizeof(T);
}

/** Map of objects based on their IDs. */
class AudioObjectMap {
public:
  typedef std::shared_ptr<AudioObject> AudioObjectPtr;

  /** Adds an object to the map.
   *
   * @param objectID Object identifier.
   * @param object The object instance to add.
   */
  static void AddObject(AudioObjectID objectID, AudioObjectPtr object);
  
  /** Finds an object based on the given ID.
   *
   * @param objectID Object identifier.
   * @return The object instance associated with the given identifier.
   * @note An exception is thrown if the object ID is not found.
   */
  static AudioObject& FindObject(AudioObjectID objectID);
  
private:
  static std::map<AudioObjectID, AudioObjectPtr> audioObjects_;
};

#endif /* AudioObject_h */
