using System;

namespace Epoch
{
    public static class SceneManager
    {
        public static void LoadScene(Scene aScene)
        {
            if (!aScene.Handle.IsValid())
            {
                Console.WriteLine("SceneManager: Tried to a scene with an invalid ID '{0}'", aScene.Handle);
                //Log.Error("SceneManager: Tried to a scene with an invalid ID '{0}'", scene.Handle);
                return;
            }

            InternalCalls.SceneManager_LoadScene(ref aScene.myHandle);
        }
    }
}
