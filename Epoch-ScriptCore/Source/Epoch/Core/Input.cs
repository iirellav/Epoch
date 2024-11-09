using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Epoch
{
    public class Input
    {
        /// <summary>
        /// Returns true the first frame that the key represented by the given KeyCode is pressed down
        /// </summary>
        public static bool IsKeyPressed(KeyCode aKeycode) => InternalCalls.Input_IsKeyPressed(aKeycode);

        /// <summary>
        /// Returns true every frame after the key was initially pressed (returns false when <see cref="Input.IsKeyPressed(KeyCode)"/> returns true)
        /// </summary>
        public static bool IsKeyHeld(KeyCode aKeycode) => InternalCalls.Input_IsKeyHeld(aKeycode);

        /// <summary>
        /// Returns true during the frame that the key was released
        /// </summary>
        public static bool IsKeyReleased(KeyCode aKeycode) => InternalCalls.Input_IsKeyReleased(aKeycode);


        /// <summary>
        /// Returns true the first frame that the button represented by the given MouseButton is pressed down
        /// </summary>
        public static bool IsMouseButtonPressed(MouseButton aButton) => InternalCalls.Input_IsMouseButtonPressed(aButton);

        /// <summary>
        /// Returns true every frame after the button was initially pressed (returns false when <see cref="Input.IsMouseButtonPressed(MouseButton)"/> returns true)
        /// </summary>
        public static bool IsMouseButtonHeld(MouseButton aButton) => InternalCalls.Input_IsMouseButtonHeld(aButton);

        /// <summary>
        /// Returns true during the frame that the button was released
        /// </summary>
        public static bool IsMouseButtonReleased(MouseButton aButton) => InternalCalls.Input_IsMouseButtonReleased(aButton);

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


        /// <summary>
        /// Returns true the first frame that the button represented by the given MouseButton is pressed down
        /// </summary>
        public static bool IsGamepadButtonPressed(GamepadButton aButton) => InternalCalls.Input_IsGamepadButtonPressed(aButton);

        /// <summary>
        /// Returns true every frame after the button was initially pressed (returns false when <see cref="Input.IsGamepadButtonPressed(GamepadButton)"/> returns true)
        /// </summary>
        public static bool IsGamepadButtonHeld(GamepadButton aButton) => InternalCalls.Input_IsGamepadButtonHeld(aButton);

        /// <summary>
        /// Returns true during the frame that the button was released
        /// </summary>
        public static bool IsGamepadButtonReleased(GamepadButton aButton) => InternalCalls.Input_IsGamepadButtonReleased(aButton);

        public static float GetGamepadAxis(GamepadAxis aAxis) => InternalCalls.Input_GetGamepadAxis(aAxis);
    }
}
