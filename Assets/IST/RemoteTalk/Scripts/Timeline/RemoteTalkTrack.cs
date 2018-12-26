#if UNITY_2017_1_OR_NEWER
using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;
using UnityEngine.SceneManagement;
#if UNITY_EDITOR
using UnityEditor;
using UnityEditor.Timeline;
#endif

namespace IST.RemoteTalk
{
    [TrackColor(0.5179334f, 0.7978405f, 0.9716981f)]
    [TrackBindingType(typeof(AudioSource))]
    [TrackClipType(typeof(RemoteTalkClip))]
    public class RemoteTalkTrack : TrackAsset
    {
        public enum ArrangeMode
        {
            None,
            OnlySelf,
            RemoteTalkTracks,
            AllTracks,
        }

        public bool fitDuration = true;
        public ArrangeMode arrangeMode = ArrangeMode.AllTracks;
        public bool pauseWhenExport = true;
        bool m_resumeRequested;

        public PlayableDirector director { get; set; }

        public AudioSource audioSource
        {
            get
            {
                if (director != null)
                {
                    foreach (var output in outputs)
                    {
                        var ret = director.GetGenericBinding(output.sourceObject) as AudioSource;
                        if (ret != null)
                            return ret;
                    }
                }
                return null;
            }

            set
            {
                if (director != null)
                    director.SetGenericBinding(this, value);
            }
        }

        public TimelineClip AddClip(Talk talk)
        {
            var ret = CreateDefaultClip();
            var asset = ret.asset as RemoteTalkClip;
            asset.talk = talk;
            asset.audioClip.defaultValue = talk.audioClip;
            return ret;
        }

        public List<TimelineClip> AddClips(IEnumerable<Talk> talks)
        {
            var ret = new List<TimelineClip>();
            foreach(var talk in talks)
                ret.Add(AddClip(talk));
            return ret;
        }

        public void ConvertToAudioTrack()
        {
            var timeline = timelineAsset;
            var audioTrack = timeline.CreateTrack<AudioTrack>(null, name);

            var output = audioSource;
            if (output != null)
                director.SetGenericBinding(audioTrack, output);

            foreach (var srcClip in GetClips())
            {
                var srcAsset = (RemoteTalkClip)srcClip.asset;
                var ac = srcAsset.audioClip.defaultValue;
                if (ac == null)
                    continue;

                var dstClip = audioTrack.CreateClip((AudioClip)ac);
                dstClip.displayName = srcClip.displayName;
                dstClip.start = srcClip.start;
                dstClip.duration = srcClip.duration;
            }
            RefreshTimelineWindow();
        }

        public void OnTalk(RemoteTalkBehaviour behaviour, FrameData info)
        {
            if (pauseWhenExport && info.evaluationType == FrameData.EvaluationType.Playback)
            {
                behaviour.director.Pause();
                m_resumeRequested = true;
            }
        }

        public void OnAudioClipUpdated(RemoteTalkBehaviour behaviour)
        {
            var clip = behaviour.clip;
            var rtc = (RemoteTalkClip)clip.asset;
            double prev = clip.duration;
            double duration = rtc.duration;
            double gap = duration - prev;

            if (fitDuration)
            {
                clip.duration = duration;
                ArrangeClips(clip.start, gap);
            }
            if (pauseWhenExport && m_resumeRequested)
            {
                director.time = clip.end;
                director.Resume();
                m_resumeRequested = false;
            }
        }

        public void EnumerateAllTracksImpl(TrackAsset track, Action<TrackAsset> act)
        {
            act(track);
            foreach (var child in track.GetChildTracks())
                EnumerateAllTracksImpl(child, act);
        }

        public void EnumerateAllTracks(Action<TrackAsset> act)
        {
            foreach (var track in timelineAsset.GetRootTracks())
                EnumerateAllTracksImpl(track, act);
        }

        public void ArrangeClips(double time, double gap)
        {
            if (arrangeMode == ArrangeMode.None)
                return;

            var tracks = new List<TrackAsset>();
            switch (arrangeMode)
            {
                case ArrangeMode.OnlySelf:
                    tracks.Add(this);
                    break;
                case ArrangeMode.RemoteTalkTracks:
                    EnumerateAllTracks(track=> {
                        var rtt = track as RemoteTalkTrack;
                        if (rtt != null)
                            tracks.Add(rtt);
                    });
                    break;
                case ArrangeMode.AllTracks:
                    EnumerateAllTracks(track => tracks.Add(track));
                    break;
                default:
                    break;
            }

            foreach(var track in tracks)
            {
                foreach (var clip in track.GetClips())
                {
                    if (clip.start > time)
                        clip.start = clip.start + gap;
                }
            }
        }

        public bool ImportText(string path)
        {
            var talks = RemoteTalkScript.TextFileToTalks(path);
            if (talks != null)
            {
                AddClips(talks);
                RefreshTimelineWindow();
                return true;
            }
            return false;
        }

        public bool ExportText(string path)
        {
            var talks = new List<Talk>();
            foreach (var c in GetClips())
            {
                var talk = ((RemoteTalkClip)c.asset).talk;
                if (talks != null)
                    talks.Add(talk);
            }
            return RemoteTalkScript.TalksToTextFile(path, talks);
        }

        public static void RefreshTimelineWindow()
        {
#if UNITY_EDITOR
#if UNITY_2018_3_OR_NEWER
            TimelineEditor.Refresh(RefreshReason.ContentsAddedOrRemoved);
#else
            // select GO that doesn't have PlayableDirector to clear timeline window
            var roots = SceneManager.GetActiveScene().GetRootGameObjects();
            foreach(var go in roots)
            {
                var director = go.GetComponent<PlayableDirector>();
                if (director == null)
                {
                    Selection.activeGameObject = go;
                    break;
                }
            }
#endif
#endif
        }


        public override Playable CreateTrackMixer(PlayableGraph graph, GameObject go, int inputCount)
        {
            director = go.GetComponent<PlayableDirector>();

            var playable = ScriptPlayable<RemoteTalkMixerBehaviour>.Create(graph, inputCount);
            var mixer = playable.GetBehaviour();
            return playable;
        }

        protected override Playable CreatePlayable(PlayableGraph graph, GameObject go, TimelineClip clip)
        {
            var ret = base.CreatePlayable(graph, go, clip);
            var playable = (ScriptPlayable<RemoteTalkBehaviour>)ret;
            var behaviour = playable.GetBehaviour();
            behaviour.director = go.GetComponent<PlayableDirector>();
            behaviour.track = this;
            behaviour.clip = clip;

            var rtc = (RemoteTalkClip)clip.asset;
            clip.displayName = rtc.talk.text + "_" + rtc.talk.castName;

            return ret;
        }
    }
}
#endif
