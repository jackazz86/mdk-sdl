using System.Runtime.InteropServices;

namespace Mdk.Engine.Platform;

/// <summary>The Switch's pad as keys and mouse motion: the native runner's <c>kg_pump_input</c> pushes
/// SDL keyboard and mouse events once a frame (the runner's static P/Invoke library "KGSwitch").
/// Off unless the runner set MDK_SWITCH=1.</summary>
public static unsafe class SwitchBridge
{
    public static readonly bool Enabled = Environment.GetEnvironmentVariable("MDK_SWITCH") == "1";

    [DllImport("KGSwitch", EntryPoint = "kg_pump_input")]
    private static extern void PumpNative(void* window);

    internal static void Pump(void* window)
    {
        if (Enabled)
        {
            PumpNative(window);
        }
    }
}
