struct IAudioLayer
{
	virtual ~IAudioLayer() {};
	virtual float GetVolume() const = 0;
	virtual void SetVolume(float volume) = 0;
};
