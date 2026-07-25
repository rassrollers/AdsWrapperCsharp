using AdsWrapper;
using Microsoft.Extensions.Configuration;

namespace AdsTestApp;

/// <summary>
/// Configures and initializes the logger with severity level filtering.
/// Supports configuration from appsettings.json with build-configuration-based defaults.
/// </summary>
public static class LoggerSetup
{
    /// <summary>
    /// Initializes the logger by loading configuration from appsettings.json,
    /// setting up filtering based on the configured log level, and registering
    /// the callback with LoggerWrapper.
    /// </summary>
    public static void Initialize()
    {
        // Load configuration from appsettings.json
        var config = new ConfigurationBuilder()
            .SetBasePath(AppContext.BaseDirectory)
            .AddJsonFile("appsettings.json", optional: false, reloadOnChange: true)
            .Build();

        // Determine default log level based on build configuration
        var defaultLogLevel = GetDefaultLogLevel();

        // Get log level from configuration (default based on build type)
        var logLevelStr = config["Logging:LogLevel:Default"] ?? defaultLogLevel;

        // Normalize standard .NET log level names to our enum names
        logLevelStr = NormalizeLogLevel(logLevelStr);

        // Parse the log level
        if (!Enum.TryParse<LogLevel>(logLevelStr, ignoreCase: true, out var configuredLogLevel))
        {
            configuredLogLevel = LogLevel.Info;
            Console.WriteLine($"Warning: Invalid log level '{logLevelStr}' in appsettings.json. Using 'Info'.");
        }

        Console.WriteLine($"Logger initialized with level: {configuredLogLevel}");
        Console.WriteLine();

        // Register a log callback that filters based on configured level
        LoggerWrapper.SetCallback((level, message) =>
        {
            // Only display logs at or above the configured severity
            if ((int)level >= (int)configuredLogLevel)
            {
                Console.WriteLine($"[{level}] {message}");
            }
        });
    }

    /// <summary>
    /// Gets the default log level based on the current build configuration.
    /// Debug builds default to Debug; Release builds default to Information.
    /// </summary>
    /// <returns>The default log level string</returns>
    private static string GetDefaultLogLevel()
    {
#if DEBUG
        return "Debug";
#else
        return "Information";
#endif
    }

    /// <summary>
    /// Normalizes standard .NET logging level names to match the LogLevel enum.
    /// </summary>
    /// <param name="logLevel">The log level string to normalize</param>
    /// <returns>The normalized log level string</returns>
    private static string NormalizeLogLevel(string logLevel)
    {
        return logLevel switch
        {
            "Information" => "Info",
            "Verbose" => "Debug",
            _ => logLevel
        };
    }
}
