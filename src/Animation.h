#ifndef ANIMATION_H
#define ANIMATION_H

#include <vector>

#include "MDXStructs.h"

typedef enum {
	AnimationInterpolationTypeNone,
	AnimationInterpolationTypeLinear,
	AnimationInterpolationTypeCount
} AnimationInterpolationType;

template <class T>
class AnimationIdentity {
	
	public:
	
		static const T& convert(const T& x) {
			return x;
		}
};

class Animation4sTo4f {
	
	public:
	
		static const Vector4f convert(const Vector4s& x) {
			return Vector4f(
				(x.x < 0 ? x.x + 32768 : x.x - 32767) / -32767.0,
				(x.y < 0 ? x.y + 32768 : x.y - 32767) / -32767.0,
				(x.z < 0 ? x.z + 32768 : x.z - 32767) / -32767.0,
				(x.w < 0 ? x.w + 32768 : x.w - 32767) / 32767.0
			);
		}
};

class Animation1sTo1f {
	
	public:
	
		static const float convert(const short& x) {
			return x / 32767.0;
		}
};

template<class T, class F = T, class C = AnimationIdentity<T> >
class Animation {

	public:
	
		Animation<T, F, C>() {
			_isInitialized = false;
		}
		
		~Animation<T, F, C>() {
		}
		
		bool initWithMDXData(MDXAnimationBlock& block, char* data, size_t size) {
			_isInitialized = false;

			if (block.times != block.keys) {
				return false;
			}

			if (size < block.timesOffset + block.times * 4 || size < block.keysOffset + block.keys * sizeof(T)) {
				return false;
			}

			// TODO: Sequence numbers / Hermite interpolation.
			
			_interpolationType = block.interpolationType;
			
			if (_interpolationType >= AnimationInterpolationTypeCount) {
				printf("WARNING: Unsupported interpolation type. Using linear instead.\n");
				_interpolationType = AnimationInterpolationTypeLinear;
			}
			
			_timelines.clear();

			bool hasFallback = false;

			MDXTimelineTimes* tt = (MDXTimelineTimes*)(data + block.timesOffset);
			MDXTimelineKeys*  tk = (MDXTimelineKeys*) (data + block.keysOffset);
			for (unsigned int i = 0; i < block.times; ++i, ++tt, ++tk) {
				if (tt->times != tk->keys) {
					return false;
				}

				_timelines.push_back(std::vector<TimelineKey>());
				
				uint32_t* t = (uint32_t*)(data + tt->timesOffset);
				F* k = (F*)(data + tk->keysOffset);
				for (unsigned int j = 0; j < tt->times; ++j, ++t, ++k) {
					if (!hasFallback) {
						_fallbackValue = C::convert(*k);
						hasFallback = true;
					}
					_timelines[i].push_back(TimelineKey(*t, C::convert(*k)));
					T v = C::convert(*k);
				}
			}
			
			_isInitialized = true;
			return true;
		}
		
		bool isAnimated(uint32_t timeline) {
			return (_isInitialized && (timeline >= _timelines.size() && _timelines.size() > 0) || (timeline < _timelines.size() && _timelines[timeline].size() >= 1));
		}
		
		T getValue(uint32_t timeline, uint32_t time) {
			if (timeline != 0 && timeline >= _timelines.size()) {
				return getValue(0, time);
			}
			if (!_isInitialized || _timelines[timeline].size() < 1 || _timelines[timeline][0].time != 0) {
				return _fallbackValue;
			}
			
			unsigned int len = _timelines[timeline].back().time - _timelines[timeline].front().time;
			
			if (_interpolationType == AnimationInterpolationTypeNone || len == 0) {
				return _timelines[timeline][0].value;
			}
			
			time %= len;
			
			for (unsigned int k = 1; k < _timelines[timeline].size(); ++k) {
				if (_timelines[timeline][k].time >= time) {
					TimelineKey k1 = _timelines[timeline][k - 1];
					TimelineKey k2 = _timelines[timeline][k];
					// linear
					return k1.value + (k2.value - k1.value) * (float)(time - k1.time) / (k2.time - k1.time);
				}
			}

			return _timelines[timeline][0].value;
		}
		
		uint32_t length(uint32_t timeline) {
			if (!_isInitialized || timeline >= _timelines.size() || _timelines[timeline].size() < 1) {
				return 0;
			}
			
			if (_interpolationType == AnimationInterpolationTypeNone) {
				return 1;
			}
			
			return (_timelines[timeline].back().time - _timelines[timeline].front().time);
		}

	private:

		struct TimelineKey {
			uint32_t time;
			T value;
			
			TimelineKey() {};
			TimelineKey(uint32_t _time, T _value) : time(_time), value(_value) {};
		};

		std::vector< std::vector<TimelineKey> > _timelines;
		bool _isInitialized;
		unsigned int _interpolationType;
		T _fallbackValue;
};

#endif