#ifndef L3_FAKE_AUDIO_BACKEND_H
#define L3_FAKE_AUDIO_BACKEND_H
#include <string>
#include <vector>
// Outside AudioBackend so tests observe the boundary, not facade internals.
inline int audioLoads=0, audioStarts=0, audioUpdates=0, audioPauses=0;
inline int audioResumes=0, audioUnloads=0, audioShots=0, audioHits=0;
inline std::vector<std::string> audioEvents;
inline bool audioLoadSucceeds = true;
inline void ResetAudioCounts() {
    audioLoads=audioStarts=audioUpdates=audioPauses=0;
    audioResumes=audioUnloads=audioShots=audioHits=0;
    audioEvents.clear();
}
namespace AudioBackend {
inline bool Load() { ++audioLoads; audioEvents.push_back("load"); return audioLoadSucceeds; }
inline void Start() { ++audioStarts; audioEvents.push_back("start"); }
inline void Update() { ++audioUpdates; audioEvents.push_back("update"); }
inline void Pause() { ++audioPauses; audioEvents.push_back("pause"); }
inline void Resume() { ++audioResumes; audioEvents.push_back("resume"); }
inline void Unload() { ++audioUnloads; audioEvents.push_back("unload"); }
inline void Shot() { ++audioShots; audioEvents.push_back("shot"); }
inline void Hit() { ++audioHits; audioEvents.push_back("hit"); }
}
#endif
