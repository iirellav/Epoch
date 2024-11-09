using System;

namespace Epoch
{
    public class Scene : IEquatable<Scene>
    {
        internal AssetHandle myHandle;
        public AssetHandle Handle => myHandle;

        internal Scene() { myHandle = AssetHandle.Invalid; }
        internal Scene(AssetHandle handle) { myHandle = handle; }


        public static Entity GetEntityByName(string aName)
        {
            ulong entityID = InternalCalls.Scene_GetEntityByName(aName);
            return entityID == 0 ? null : new Entity(entityID);


            //ulong hashCode = (ulong)aName.GetHashCode();
            //
            //Entity result = null;
            //
            //if (s_EntityCache.ContainsKey(hashCode) && s_EntityCache[hashCode] != null)
            //{
            //    result = s_EntityCache[hashCode];
            //    if (!InternalCalls.Scene_IsEntityValid(result.ID))
            //    {
            //        s_EntityCache.Remove(hashCode);
            //        result = null;
            //    }
            //}
            //
            //if (result == null)
            //{
            //    ulong entityID = InternalCalls.Scene_FindEntityByTag(tag);
            //    Entity newEntity = entityID != 0 ? new Entity(entityID) : null;
            //    s_EntityCache[hashCode] = newEntity;
            //
            //    if (newEntity != null)
            //        newEntity.DestroyedEvent += OnEntityDestroyed;
            //
            //    result = newEntity;
            //}
            //
            //return result;
        }

        public static Entity GetEntityByID(ulong entityID)
        {
            if (!InternalCalls.Scene_IsEntityValid(entityID)) return null;

            Entity newEntity = new Entity(entityID);
            return newEntity;
        }

        public static Entity CreateEntity(string aName)
        {
            ulong entityID = InternalCalls.Scene_CreateEntity(aName);
            return entityID == 0 ? null : new Entity(entityID);
        }

        public static void DestroyEntity(Entity aEntity)
        {
            if (aEntity == null)
            {
                return;
            }

            if (!InternalCalls.Scene_IsEntityValid(aEntity.id))
            {
                return;
            }

            InternalCalls.Scene_DestroyEntity(aEntity.id);
        }

        public static void DestroyAllChildren(Entity aEntity)
        {
            if (aEntity == null)
            {
                return;
            }

            if (!InternalCalls.Scene_IsEntityValid(aEntity.id))
            {
                return;
            }
            
            InternalCalls.Scene_DestroyAllChildren(aEntity.id);
        }


        public static Entity InstantiatePrefab(Prefab aPrefab)
        {
            ulong entityID = InternalCalls.Scene_InstantiatePrefab(ref aPrefab.myHandle);
            return entityID == 0 ? null : new Entity(entityID);
        }

        public static Entity InstantiatePrefab(Prefab aPrefab, Vector3 aTranslation)
        {
            ulong entityID = InternalCalls.Scene_InstantiatePrefabWithTranslation(ref aPrefab.myHandle, ref aTranslation);
            return entityID == 0 ? null : new Entity(entityID);
        }

        public static Entity InstantiatePrefab(Prefab aPrefab, Vector3 aTranslation, Vector3 aRotation)
        {
            ulong entityID = InternalCalls.Scene_InstantiatePrefabWithTranslationAndRotation(ref aPrefab.myHandle, ref aTranslation, ref aRotation);
            return entityID == 0 ? null : new Entity(entityID);
        }

        public static Entity InstantiatePrefab(Prefab aPrefab, Vector3 aTranslation, Vector3 aRotation, Vector3 aScale)
        {
            ulong entityID = InternalCalls.Scene_InstantiatePrefabWithTransform(ref aPrefab.myHandle, ref aTranslation, ref aRotation, ref aScale);
            return entityID == 0 ? null : new Entity(entityID);
        }

        public static Entity InstantiatePrefab(Prefab aPrefab, Transform aTransform)
        {
            ulong entityID = InternalCalls.Scene_InstantiatePrefabWithTransform(ref aPrefab.myHandle, ref aTransform.position, ref aTransform.rotation, ref aTransform.scale);
            return entityID == 0 ? null : new Entity(entityID);
        }


        public static Entity InstantiatePrefabWithParent(Prefab aPrefab, Entity aParent)
        {
            ulong entityID = InternalCalls.Scene_InstantiatePrefabWithParent(ref aPrefab.myHandle, aParent.id);
            return entityID == 0 ? null : new Entity(entityID);
        }

        public static Entity InstantiatePrefabWithParent(Prefab aPrefab, Entity aParent, Vector3 aTranslation)
        {
            ulong entityID = InternalCalls.Scene_InstantiatePrefabWithTranslationWithParent(ref aPrefab.myHandle, aParent.id, ref aTranslation);
            return entityID == 0 ? null : new Entity(entityID);
        }

        public static Entity InstantiatePrefabWithParent(Prefab aPrefab, Entity aParent, Vector3 aTranslation, Vector3 aRotation)
        {
            ulong entityID = InternalCalls.Scene_InstantiatePrefabWithTranslationAndRotationWithParent(ref aPrefab.myHandle, aParent.id, ref aTranslation, ref aRotation);
            return entityID == 0 ? null : new Entity(entityID);
        }

        public static Entity InstantiatePrefabWithParent(Prefab aPrefab, Entity aParent, Vector3 aTranslation, Vector3 aRotation, Vector3 aScale)
        {
            ulong entityID = InternalCalls.Scene_InstantiatePrefabWithTransformWithParent(ref aPrefab.myHandle, aParent.id, ref aTranslation, ref aRotation, ref aScale);
            return entityID == 0 ? null : new Entity(entityID);
        }

        public static Entity InstantiatePrefabWithParent(Prefab aPrefab, Entity aParent, Transform aTransform)
        {
            ulong entityID = InternalCalls.Scene_InstantiatePrefabWithTransformWithParent(ref aPrefab.myHandle, aParent.id, ref aTransform.position, ref aTransform.rotation, ref aTransform.scale);
            return entityID == 0 ? null : new Entity(entityID);
        }


        public override bool Equals(object obj) => obj is Scene other && Equals(other);

        public bool Equals(Scene other)
        {
            if (other is null)
            {
                return false;
            }

            if (ReferenceEquals(this, other))
            {
                return true;
            }

            return myHandle == other.myHandle;
        }
        private static bool IsValid(Scene scene)
        {
            if (scene is null)
            {
                return false;
            }

            return scene.myHandle.IsValid();
        }

        public override int GetHashCode() => myHandle.GetHashCode();

        public static bool operator ==(Scene aSceneA, Scene aSceneB) => aSceneA is null ? aSceneB is null : aSceneA.Equals(aSceneB);
        public static bool operator !=(Scene aSceneA, Scene aSceneB) => !(aSceneA == aSceneB);

        public static implicit operator bool(Scene aScene) => IsValid(aScene);
    }
}
