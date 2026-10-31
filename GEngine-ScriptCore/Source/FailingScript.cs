using System;

namespace GEngine
{
    public sealed class FailingScript : ScriptBehaviour
    {
        public override void OnStart()
        {
            Console.WriteLine("[FailingScript] Start");
        }

        public override void OnUpdate(float deltaTime)
        {
            throw new InvalidOperationException("Expected script failure.");
        }

        public override void OnDestroy()
        {
            Console.WriteLine("[FailingScript] Destroy");
        }
    }
}
