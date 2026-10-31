using System;

namespace GEngine
{
    public sealed class WorldLogicScript : ScriptBehaviour
    {
        private int fixedUpdates;
        private int updates;

        public override void OnAwake()
        {
            Console.WriteLine("[Script] Awake {0}:{1}:{2}", GameObject.UUID, GameObject.Name, GameObject.Active);
        }

        public override void OnStart()
        {
            Console.WriteLine("[Script] Start {0}:{1}", GameObject.UUID, GameObject.Name);
        }

        public override void OnEnable()
        {
            Console.WriteLine("[Script] Enable {0}:{1}:{2}", GameObject.UUID, GameObject.Name, GameObject.Active);
        }

        public override void OnDisable()
        {
            Console.WriteLine("[Script] Disable {0}:{1}:{2}", GameObject.UUID, GameObject.Name, GameObject.Active);
        }

        public override void OnFixedUpdate(float fixedDeltaTime)
        {
            ++fixedUpdates;
            if (fixedUpdates == 1)
                Console.WriteLine("[Script] Fixed {0:F3}", fixedDeltaTime);
        }

        public override void OnUpdate(float deltaTime)
        {
            ++updates;
            if (updates == 1)
                Console.WriteLine("[Script] Update {0:F3}", deltaTime);
        }

        public override void OnLateUpdate()
        {
            if (updates == 1)
                Console.WriteLine("[Script] LateUpdate");
        }

        public override void OnPause()
        {
            Console.WriteLine("[Script] Pause");
        }

        public override void OnResume()
        {
            Console.WriteLine("[Script] Resume");
        }

        public override void OnDestroy()
        {
            Console.WriteLine("[Script] Destroy fixed={0} update={1}", fixedUpdates, updates);
        }
    }
}
