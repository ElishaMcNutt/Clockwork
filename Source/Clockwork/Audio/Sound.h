#pragma once

#include "../Container/ArrayPtr.h"
#include "../Resource/Resource.h"

namespace Clockwork
{

class SoundStream;

/// %Sound resource.
class CLOCKWORK_API Sound : public Resource
{
    OBJECT(Sound);

public:
    /// Construct.
    Sound(Context* context);
    /// Destruct and free sound data.
    virtual ~Sound();
    /// Register object factory.
    static void RegisterObject(Context* context);

    /// Load resource from stream. May be called from a worker thread. Return true if successful.
    virtual bool BeginLoad(Deserializer& source);

    /// Load raw sound data.
    bool LoadRaw(Deserializer& source);
    /// Load WAV format sound data.
    bool LoadWav(Deserializer& source);
    /// Load Ogg Vorbis format sound data. Does not decode at load, but will rather be decoded while playing.
    bool LoadOggVorbis(Deserializer& source);
    /// Set sound size in bytes. Also resets the sound to be uncompressed and one-shot.
    void SetSize(unsigned dataSize);
    /// Set uncompressed sound data.
    void SetData(const void* data, unsigned dataSize);
    /// Set uncompressed sound data format.
    void SetFormat(unsigned frequency, bool sixteenBit, bool stereo);
    /// Set loop on/off. If loop is enabled, sets the full sound as loop range.
    void SetLooped(bool enable);
    /// Define loop.
    void SetLoop(unsigned repeatOffset, unsigned endOffset);

    /// Return a new instance of a decoder sound stream. Used by compressed sounds.
    SharedPtr<SoundStream> GetDecoderStream() const;
    /// Return shared sound data.
    SharedArrayPtr<signed char> GetData() const { return data_; }
    /// Return sound data start.
    signed char* GetStart() const { return data_.Get(); }
    /// Return loop start.
    signed char* GetRepeat() const { return repeat_; }
    /// Return sound data end.
    signed char* GetEnd() const { return end_; }
    /// Return length in seconds.
    float GetLength() const;
    /// Return total sound data size.
    unsigned GetDataSize() const { return dataSize_; }
    /// Return sample size.
    unsigned GetSampleSize() const;
    /// Return default frequency as a float.
    float GetFrequency() const { return (float)frequency_; }
    /// Return default frequency as an integer.
    unsigned GetIntFrequency() const { return frequency_; }
    /// Return whether is looped.
    bool IsLooped() const { return looped_; }
    /// Return whether data is sixteen bit.
    bool IsSixteenBit() const { return sixteenBit_; }
    /// Return whether data is stereo.
    bool IsStereo() const { return stereo_; }
    /// Return whether is compressed.
    bool IsCompressed() const { return compressed_; }

    /// Fix interpolation by copying data from loop start to loop end (looped), or adding silence (oneshot.) Called internally, does not normally need to be called, unless the sound data is modified manually on the fly.
    void FixInterpolation();

private:
    /// Load optional parameters from an XML file.
    void LoadParameters();

    /// Sound data.
    SharedArrayPtr<signed char> data_;
    /// Loop start.
    signed char* repeat_;
    /// Sound data end.
    signed char* end_;
    /// Sound data size in bytes.
    unsigned dataSize_;
    /// Default frequency.
    unsigned frequency_;
    /// Looped flag.
    bool looped_;
    /// Sixteen bit flag.
    bool sixteenBit_;
    /// Stereo flag.
    bool stereo_;
    /// Compressed flag.
    bool compressed_;
    /// Compressed sound length.
    float compressedLength_;
};

}
