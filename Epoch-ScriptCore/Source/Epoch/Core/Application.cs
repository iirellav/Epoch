using System;

namespace Epoch
{
    public struct Application
    {
        public static void Quit() => InternalCalls.Application_Quit();

        public static bool VSync
        {
            get => InternalCalls.Application_GetIsVSync();
            set => InternalCalls.Application_SetIsVSync(value);
        }

        public static uint Width() => InternalCalls.Application_GetWidth();
        public static uint Height() => InternalCalls.Application_GetHeight();
    }
}
