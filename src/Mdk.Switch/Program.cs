using Mdk.Engine.Platform;
using Mdk.Formats;
using Mdk.Game;
using Mdk.Game.Flow;
using Mdk.Game.Menu;
using Mdk.Game.Scripts;

// MDK on the Switch (NroForge: Mono full AOT, SDL3 over Vulkan/NVK). The native runner sets
// MDK_DATA_DIR (the original game's folder on the SD card) and MDK_SWITCH=1 before this starts,
// and the pad is turned into keys and mouse motion by KGSwitch.kg_pump_input (SwitchBridge).
// Mono on the Switch has no process path, so AppContext.BaseDirectory is empty: point it at the app's
// SD folder (settings, saves and mdk_paths.cfg live there).
var root = Environment.GetEnvironmentVariable("NROFORGE_ROOT");
AppContext.SetData("APP_CONTEXT_BASE_DIRECTORY", (string.IsNullOrEmpty(root) ? "/switch/mdk" : root) + "/");

if (string.IsNullOrEmpty(Environment.GetEnvironmentVariable("HOME")))
{
    Environment.SetEnvironmentVariable("HOME", (string.IsNullOrEmpty(root) ? "/switch/mdk" : root) + "/UserData");
}

var data = MdkData.Find();
if (data == null)
{
    Console.Error.WriteLine("MDK data not found: copy TRAVERSE, MISC, FALL3D and STREAM from the game to /switch/mdk/MDK");
    return 1;
}

// Test options (a PC run): --level=N starts that level, --screenshot=file.bmp --wait=seconds saves a frame.
var options = args.Where(a => a.StartsWith("--")).Select(a => a[2..].Split('=', 2)).ToDictionary(p => p[0], p => p.Length > 1 ? p[1] : "");
var level = options.TryGetValue("level", out var levelText) ? int.Parse(levelText) : 7;
options.TryGetValue("screenshot", out var screenshot);
var wait = options.TryGetValue("wait", out var waitText) ? float.Parse(waitText, System.Globalization.CultureInfo.InvariantCulture) : 0f;
float Seconds(string name) => options.TryGetValue(name, out var t) ? float.Parse(t, System.Globalization.CultureInfo.InvariantCulture) : 0f;
var viewer = new ViewerOptions(level, null, null, 0f, options.ContainsKey("mute") ? SoundMode.Muted : SoundMode.On)
{
    Screenshot = screenshot,
    Wait = wait,
    Delay = Seconds("delay"),
    Use = options.ContainsKey("use"),
    Give = options.TryGetValue("give", out var give) ? give.Split(',') : [],
    Teleport = options.TryGetValue("teleport", out var teleport) ? TeleportTarget.Parse(teleport) : null,
};
var start = options.ContainsKey("level") ? Start.Level : Start.Menu;
using var game = new Game(data, new GameOptions(start, viewer)
{
    Display = Display.Window,
    Page = MenuPage.Main,
    Splash = start == Start.Menu,
    Gpu = "vulkan",
    Screenshot = screenshot,
    Wait = wait,
});
game.Run();
return 0;
