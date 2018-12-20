#if UNITY_EDITOR
using System.Linq;
using UnityEngine;
using UnityEditor;
using UnityEditorInternal;

namespace IST.RemoteTalk
{
    [CustomEditor(typeof(RemoteTalkScript))]
    public class RemoteTalkScriptEditor : Editor
    {
        ReorderableList m_rlist;

        void OnEnable()
        {
            m_rlist = RemoteTalkEditorUtils.CreateTalkList(serializedObject, serializedObject.FindProperty("m_talks"));
        }

        public override void OnInspectorGUI()
        {
            //DrawDefaultInspector();

            var t = target as RemoteTalkScript;
            var so = serializedObject;

            serializedObject.Update();
            m_rlist.DoLayoutList();

            var talks = serializedObject.FindProperty("m_talks");
            var startPos = serializedObject.FindProperty("m_startPos");
            EditorGUILayout.PropertyField(serializedObject.FindProperty("m_playOnStart"));

            if (talks.arraySize > 0)
            {
                EditorGUI.BeginChangeCheck();
                EditorGUILayout.IntSlider(startPos, 0, talks.arraySize - 1);
                if (EditorGUI.EndChangeCheck())
                    m_rlist.index = startPos.intValue;
            }
            if (t.isPlaying)
            {
                m_rlist.index = t.playPosition >= talks.arraySize ? -1 : t.playPosition;
            }
            serializedObject.ApplyModifiedProperties();


            EditorGUILayout.Space();

            if(t.isPlaying)
            {
                if (GUILayout.Button("Stop"))
                    t.Stop();
            }
            else
            {
                if (GUILayout.Button("Play"))
                    t.Play();
            }

            EditorGUILayout.Space();

            EditorGUILayout.BeginHorizontal();
            if (GUILayout.Button("Import"))
                t.ImportFile(EditorUtility.OpenFilePanel("Import Script", ".", "txt"));
            if (GUILayout.Button("Export"))
                t.ExportFile(EditorUtility.SaveFilePanel("Export Script", ".", t.name, "txt"));
            EditorGUILayout.EndHorizontal();

            EditorGUILayout.Space();

            if (GUILayout.Button("Convert To Timeline Track"))
                t.ConvertToRemoteTalkTrack();
        }
    }
}
#endif
