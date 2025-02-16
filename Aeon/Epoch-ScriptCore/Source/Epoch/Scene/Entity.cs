using System;
using System.Collections.Generic;

namespace Epoch
{
    public class Entity : IEquatable<Entity>
    {
        protected Entity() { id = 0; }

        internal Entity(ulong aId)
        {
            id = aId;
        }

        public readonly ulong id;

        public string Name() => InternalCalls.NameComponent_GetName(id);

        public bool GetIsActive() => InternalCalls.Entity_GetIsActive(id);
        public void SetIsActive(bool aState) => InternalCalls.Entity_SetIsActive(id, aState);

        private Dictionary<Type, Component> myComponentCache = new Dictionary<Type, Component>();

        private static bool IsValid(Entity aEntity)
        {
            if (aEntity is null)
            {
                return false;
            }

            if (aEntity.id == 0)
            {
                return false;
            }

            return InternalCalls.Scene_IsEntityValid(aEntity.id);
        }


        protected virtual void OnUpdate() { }
        protected virtual void OnLateUpdate() { }
        protected virtual void OnFixedUpdate() { }

        protected virtual void OnStart() { }
        protected virtual void OnEnd() { }

        protected virtual void OnCreate() { }
        protected virtual void OnDestroy() { }

        protected virtual void OnDebug() { }

        private void OnDestroyInternal()
        {
            OnDestroy();
            myComponentCache.Clear();
        }

        //Colliders only callbacks
        protected virtual void OnCollisionEnter(ulong aID) { }
        protected virtual void OnCollisionExit(ulong aID) { }

        protected virtual void OnTriggerEnter(ulong aID) { }
        protected virtual void OnTriggerExit(ulong aID) { }

        //Meshes only callbacks
        protected virtual void OnFrustumEnter() { }
        protected virtual void OnFrustumExit() { }

        //Button only callbacks
        protected virtual void OnClick() { }
        protected virtual void OnMouseEnter() { }
        protected virtual void OnMouseExit() { }


        public T AddComponent<T>() where T : Component, new()
        {
            if (HasComponent<T>())
            {
                return GetComponent<T>();
            }

            Type componentType = typeof(T);
            InternalCalls.Entity_AddComponent(id, componentType);
            T component = new T { entity = this };
            myComponentCache.Add(componentType, component);
            return component;
        }

        public bool HasComponent<T>() where T : Component => InternalCalls.Entity_HasComponent(id, typeof(T));

        public bool RemoveComponent<T>() where T : Component
        {
            Type componentType = typeof(T);
            bool removed = InternalCalls.Entity_RemoveComponent(id, componentType);

            if (removed && myComponentCache.ContainsKey(componentType))
            {
                myComponentCache.Remove(componentType);
            }

            return removed;
        }

        public T GetComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);

            if (!HasComponent<T>())
            {
                if (myComponentCache.ContainsKey(componentType))
                {
                    myComponentCache.Remove(componentType);
                }

                return null;
            }

            if (!myComponentCache.ContainsKey(componentType))
            {
                T component = new T { entity = this };
                myComponentCache.Add(componentType, component);
                return component;
            }

            return myComponentCache[componentType] as T;
        }

        public bool TryGetComponent<T>(out T aOut) where T : Component, new()
        {
            var component = GetComponent<T>();
            if (component == null)
            {
                aOut = null;
                return false;
            }

            aOut = component;
            return true;
        }


        private TransformComponent myTransformComponent;
        public TransformComponent Transform
        {
            get
            {
                if (myTransformComponent == null)
                {
                    myTransformComponent = GetComponent<TransformComponent>();
                }

                return myTransformComponent;
            }
        }


        public Entity InstantiateChild(Prefab aPrefab) => Scene.InstantiatePrefabWithParent(aPrefab, this);
        public Entity InstantiateChild(Prefab aPrefab, Vector3 aTranslation) => Scene.InstantiatePrefabWithParent(aPrefab, this, aTranslation);
        public Entity InstantiateChild(Prefab aPrefab, Vector3 aTranslation, Vector3 aRotation) => Scene.InstantiatePrefabWithParent(aPrefab, this, aTranslation, aRotation);
        public Entity InstantiateChild(Prefab aPrefab, Vector3 aTranslation, Vector3 aRotation, Vector3 aScale) => Scene.InstantiatePrefabWithParent(aPrefab, this, aTranslation, aRotation, aScale);
        public Entity InstantiateChild(Prefab aPrefab, Transform aTransform) => Scene.InstantiatePrefabWithParent(aPrefab, this, aTransform);


        public void Destroy() => Scene.DestroyEntity(this);
        public void Destroy(Entity aOther) => Scene.DestroyEntity(aOther);
        public void DestroyAllChildren() => Scene.DestroyAllChildren(this);


        public Entity Parent
        {
            get
            {
                ulong parentID = InternalCalls.Entity_GetParent(id);
                return parentID == 0 ? null : new Entity(parentID);
            }

            set => InternalCalls.Entity_SetParent(id, value.id);
        }

        public Entity[] Children => InternalCalls.Entity_GetChildren(id);

        public Entity GetChildByName(string aName)
        {
            ulong childID = InternalCalls.Entity_GetChildByName(id, aName);
            return childID == 0 ? null : new Entity(childID);
        }


        /// <summary>
        /// Returns true if it possible to treat this entity as if it were of type T,
        /// returns false otherwise.
        /// <returns></returns>
        public bool Is<T>() where T : Entity
        {
            if (!HasComponent<ScriptComponent>())
            {
                return false;
            }

            ScriptComponent sc = GetComponent<ScriptComponent>();
            object instance = sc?.Instance;
            return instance is T;
        }

        /// <summary>
        /// Returns this entity cast to type T if that is possible, otherwise returns null.
        /// <returns></returns>
        public T As<T>() where T : Entity
        {
            ScriptComponent sc = GetComponent<ScriptComponent>();
            object instance = sc?.Instance;
            return instance as T;
        }


        public override bool Equals(object aObj) => aObj is Entity aOther && Equals(aOther);

        // https://docs.microsoft.com/en-us/dotnet/csharp/programming-guide/statements-expressions-operators/how-to-define-value-equality-for-a-type
        public bool Equals(Entity aOther)
        {
            if (aOther is null)
            {
                return false;
            }

            if (ReferenceEquals(this, aOther))
            {
                return true;
            }

            return id == aOther.id;
        }

        public override int GetHashCode() => (int)id;

        public static bool operator ==(Entity aEntityA, Entity aEntityB) => aEntityA is null ? aEntityB is null : aEntityA.Equals(aEntityB);
        public static bool operator !=(Entity aEntityA, Entity aEntityB) => !(aEntityA == aEntityB);
        public static implicit operator bool(Entity aEntity) => IsValid(aEntity);
    }
}
