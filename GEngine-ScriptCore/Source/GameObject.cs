namespace GEngine
{
    public sealed class GameObject
    {
        internal ulong uuid;
        internal string name;
        internal bool active;

        internal GameObject(ulong uuid, string name, bool active)
        {
            this.uuid = uuid;
            this.name = name;
            this.active = active;
        }

        internal void Update(string name, bool active)
        {
            this.name = name;
            this.active = active;
        }

        public ulong UUID => uuid;
        public string Name => name;
        public bool Active => active;
    }
}
