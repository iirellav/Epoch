using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Epoch
{
    public class Input
    {
        public static bool IsKeyPressed(KeyCode aKeycode) => InternalCalls.Input_IsKeyPressed(aKeycode);

        public static bool IsKeyHeld(KeyCode aKeycode) => InternalCalls.Input_IsKeyHeld(aKeycode);

        public static bool IsKeyReleased(KeyCode aKeycode) => InternalCalls.Input_IsKeyReleased(aKeycode);


        public static bool IsMouseButtonPressed(MouseButton aButton) => InternalCalls.Input_IsMouseButtonPressed(aButton);

        public static bool IsMouseButtonHeld(MouseButton aButton) => InternalCalls.Input_IsMouseButtonHeld(aButton);

        public static bool IsMouseButtonReleased(MouseButton aButton) => InternalCalls.Input_IsMouseButtonReleased(aButton);

        public static Vector2 GetMousePosition()
        {
            InternalCalls.Input_GetMousePosition(out Vector2 position);
            return position;
        }

        public static Vector2 GetMouseDelta()
        {
            InternalCalls.Input_GetMouseDelta(out Vector2 result);
            return result;
        }

        public static CursorMode GetCursorMode() => InternalCalls.Input_GetCursorMode();
        public static void SetCursorMode(CursorMode aMode) => InternalCalls.Input_SetCursorMode(aMode);

        public static Vector2 GetScrollDelta()
        {
            InternalCalls.Input_GetScrollDelta(out Vector2 result);
            return result;
        }


        public static bool IsGamepadButtonPressed(GamepadButton aButton) => InternalCalls.Input_IsGamepadButtonPressed(aButton);

        public static bool IsGamepadButtonHeld(GamepadButton aButton) => InternalCalls.Input_IsGamepadButtonHeld(aButton);

        public static bool IsGamepadButtonReleased(GamepadButton aButton) => InternalCalls.Input_IsGamepadButtonReleased(aButton);

        public static float GetGamepadAxis(GamepadAxis aAxis) => InternalCalls.Input_GetGamepadAxis(aAxis);
    }
}
