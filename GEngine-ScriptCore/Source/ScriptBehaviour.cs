namespace GEngine
{
    public class ScriptBehaviour
    {
#pragma warning disable 0649
        internal GameObject GameObject;
#pragma warning restore 0649

        public virtual void OnAwake() { }
        public virtual void OnStart() { }
        public virtual void OnEnable() { }
        public virtual void OnDisable() { }
        public virtual void OnFixedUpdate(float fixedDeltaTime) { }
        public virtual void OnUpdate(float deltaTime) { }
        public virtual void OnLateUpdate() { }
        public virtual void OnPause() { }
        public virtual void OnResume() { }
        public virtual void OnDestroy() { }
    }
}
