using System.Collections.ObjectModel;

namespace AdsTestApp;

public sealed class ConsoleMenu
{
    private readonly List<MenuEntry> _entries = new();
    private readonly Dictionary<string, MenuEntry> _entryLookup = new(StringComparer.OrdinalIgnoreCase);

    public ConsoleMenu(string title)
    {
        Title = string.IsNullOrWhiteSpace(title) ? "Menu" : title;
    }

    public string Title { get; }

    public ReadOnlyCollection<MenuEntry> Entries => _entries.AsReadOnly();

    public ConsoleMenu AddOption(string key, string description, Func<CancellationToken, Task> action, bool closeMenu = false)
    {
        if (string.IsNullOrWhiteSpace(key))
        {
            throw new ArgumentException("Menu option key is required.", nameof(key));
        }

        if (string.IsNullOrWhiteSpace(description))
        {
            throw new ArgumentException("Menu option description is required.", nameof(description));
        }

        if (action is null)
        {
            throw new ArgumentNullException(nameof(action));
        }

        if (_entryLookup.ContainsKey(key))
        {
            throw new InvalidOperationException($"A menu option with key '{key}' already exists.");
        }

        var entry = new MenuEntry(key, description, action, closeMenu);
        _entries.Add(entry);
        _entryLookup[key] = entry;
        return this;
    }

    public async Task RunAsync(CancellationToken cancellationToken = default)
    {
        if (_entries.Count == 0)
        {
            Console.WriteLine("No menu options configured.");
            return;
        }

        while (!cancellationToken.IsCancellationRequested)
        {
            DrawMenu();
            var input = Console.ReadLine()?.Trim();

            if (string.IsNullOrWhiteSpace(input))
            {
                Console.WriteLine("Please enter a menu option.");
                continue;
            }

            if (!_entryLookup.TryGetValue(input, out var entry))
            {
                Console.WriteLine($"Unknown option '{input}'.");
                continue;
            }

            try
            {
                await entry.Action(cancellationToken);
            }
            catch (OperationCanceledException) when (cancellationToken.IsCancellationRequested)
            {
                Console.WriteLine("Operation cancelled.");
                break;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Option '{entry.Key}' failed: {ex.Message}");
            }

            if (entry.CloseMenu)
            {
                break;
            }
        }
    }

    private void DrawMenu()
    {
        Console.WriteLine();
        Console.WriteLine(Title);
        foreach (var entry in _entries)
        {
            Console.WriteLine($"{entry.Key}: {entry.Description}");
        }
        Console.Write("Select option: ");
    }
}

public sealed record MenuEntry(
    string Key,
    string Description,
    Func<CancellationToken, Task> Action,
    bool CloseMenu);
