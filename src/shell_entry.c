/**
 * shell_entry.c - Unified entry point for the shell application
 *
 * This file combines the functionality of:
 * - main.c (standard command-line mode)
 * - main_gui.c (Windows GUI mode with WinMain)
 * - win_main.c (Windows-specific implementation)
 * - win_gui_common.c (common GUI functions)
 *
 * The application can run in either:
 * 1. Console mode (standard shell interface)
 * 2. GUI mode (Windows GUI with embedded shell)
 */

#include "shell.h"

#ifdef WINDOWS
    #include <windows.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <fcntl.h>
    #include <io.h>
    #include <shellapi.h>
    #include <commctrl.h>  /* For common controls */
    #define popen _popen
    #define pclose _pclose
    #define fileno _fileno
    #define isatty _isatty
#endif

/* Resource IDs */
#define IDI_SHELL_ICON        101
#define IDR_MENU_MAIN         102
#define IDC_STATUSBAR         103
#define IDC_TOOLBAR           104
#define IDC_CONSOLE_CONTAINER 105

/* Menu IDs */
#define ID_FILE_EXIT          201
#define ID_EDIT_CLEAR         202
#define ID_VIEW_FONT          203
#define ID_HELP_ABOUT         204

/* Window dimensions */
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 700

/* Global variables for GUI mode */
#ifdef WINDOWS
    BOOL g_GuiMode = FALSE;           /* Flag to indicate GUI mode is active */
    HWND g_hMainWindow = NULL;        /* Main window handle */
    HWND g_hStatusBar = NULL;         /* Status bar window handle */
    HWND g_hToolBar = NULL;           /* Toolbar window handle */
    HWND g_hConsoleContainer = NULL;  /* Container for console window */
    HANDLE g_hConsoleIn = NULL;       /* Console input handle */
    HANDLE g_hConsoleOut = NULL;      /* Console output handle */
    HFONT g_hFont = NULL;             /* Font handle */
    HBRUSH g_hBackgroundBrush = NULL; /* Background brush */

    /* Forward declarations */
    int shell_main(int argc, char *argv[]);
    LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    DWORD WINAPI ShellThreadProc(LPVOID lpParameter);
    BOOL setup_console_window(BOOL force_visible);
    char **convert_args(int argc, LPWSTR *wargv);
    void free_args(int argc, char **argv);
    HWND create_status_bar(HWND hParent);
    HWND create_toolbar(HWND hParent);
    BOOL WINAPI ConsoleEventHandler(DWORD dwCtrlType);
    int initialize_console_for_gui(void);
#endif

/* The init_locale function is defined in locale.c */

/**
 * shell_main - Main shell logic (formerly in main.c)
 * @argc: argument count
 * @argv: argument vector
 *
 * Return: 0 on success, 1 on error
 */
int shell_main(int argc, char *argv[])
{
    info_t info[] = { INFO_INIT };
    int fd = 2;

    // Initialize locale for better internationalization support
    init_locale();
    
    // Display welcome message in the current language
    if (get_language() == 1) /* LANG_AR */
        _puts_utf8((char *)get_message(MSG_WELCOME));
    else
        _puts((char *)get_message(MSG_WELCOME));
    _putchar('\n');

#ifdef WINDOWS
    // Windows specific initialization - already handled in configure_terminal_for_utf8
#endif

#ifdef WINDOWS
    // Windows doesn't support inline assembly in MSVC
    // Just skip this part
#else
    asm ("mov %1, %0\n\t"
        "add $3, %0"
        : "=r" (fd)
        : "r" (fd));
#endif

    if (argc == 2)
    {
        fd = open(argv[1], O_RDONLY);
        if (fd == -1)
        {
            if (errno == EACCES)
                exit(126);
            if (errno == ENOENT)
            {
                _eputs(argv[0]);
                _eputs(": 0: Can't open ");
                _eputs(argv[1]);
                _eputchar('\n');
                _eputchar(BUF_FLUSH);
                exit(127);
            }
            return (EXIT_FAILURE);
        }
        info->readfd = fd;
    }
    populate_env_list(info);
    read_history(info);
    hsh(info, argv);
    return (EXIT_SUCCESS);
}

#ifdef WINDOWS

/**
 * ConsoleEventHandler - Handles console control events
 * @dwCtrlType: The type of control event
 *
 * Return: TRUE if the event was handled, FALSE otherwise
 */
BOOL WINAPI ConsoleEventHandler(DWORD dwCtrlType)
{
    switch (dwCtrlType)
    {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
            /* Handle console control events */
            if (g_GuiMode)
            {
                /* In GUI mode, don't exit on Ctrl+C */
                return TRUE;
            }
            return FALSE;
        default:
            return FALSE;
    }
}

/**
 * create_status_bar - Creates the status bar
 * @hParent: Parent window handle
 *
 * Return: The status bar window handle
 */
HWND create_status_bar(HWND hParent)
{
    HWND hStatus;
    int statusParts[] = {200, 400, 600, -1};

    /* Create the status bar */
    hStatus = CreateWindowEx(
        0,
        STATUSCLASSNAME,
        NULL,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,
        hParent,
        (HMENU)IDC_STATUSBAR,
        GetModuleHandle(NULL),
        NULL);

    /* Set the status bar parts */
    SendMessage(hStatus, SB_SETPARTS, 4, (LPARAM)statusParts);

    /* Set the text for each part */
    SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"Ready");
    SendMessage(hStatus, SB_SETTEXT, 1, (LPARAM)"UTF-8");
    SendMessage(hStatus, SB_SETTEXT, 2, (LPARAM)"Simple Shell");
    SendMessage(hStatus, SB_SETTEXT, 3, (LPARAM)"Arabic Support");

    return hStatus;
}

/**
 * create_toolbar - Creates the toolbar
 * @hParent: Parent window handle
 *
 * Return: The toolbar window handle
 */
HWND create_toolbar(HWND hParent)
{
    /* Toolbar buttons */
    TBBUTTON tbb[] = {
        {0, ID_FILE_EXIT, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, (INT_PTR)"Exit"},
        {0, 0, TBSTATE_ENABLED, BTNS_SEP, {0}, 0, 0},
        {1, ID_EDIT_CLEAR, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, (INT_PTR)"Clear"},
        {0, 0, TBSTATE_ENABLED, BTNS_SEP, {0}, 0, 0},
        {2, ID_VIEW_FONT, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, (INT_PTR)"Font"},
        {0, 0, TBSTATE_ENABLED, BTNS_SEP, {0}, 0, 0},
        {3, ID_HELP_ABOUT, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, (INT_PTR)"About"}
    };

    /* Create the toolbar */
    HWND hToolBar = CreateWindowEx(
        0,
        TOOLBARCLASSNAME,
        NULL,
        WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS,
        0, 0, 0, 0,
        hParent,
        (HMENU)IDC_TOOLBAR,
        GetModuleHandle(NULL),
        NULL);

    /* Set the toolbar buttons */
    SendMessage(hToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(hToolBar, TB_ADDBUTTONS, (WPARAM)7, (LPARAM)&tbb);
    SendMessage(hToolBar, TB_AUTOSIZE, 0, 0);
    
    return hToolBar;
}

/**
 * setup_console_window - Set up console window with proper settings
 * @force_visible: Force the console window to be visible
 *
 * Return: TRUE if successful, FALSE otherwise
 */
BOOL setup_console_window(BOOL force_visible)
{
    BOOL result = TRUE;
    HANDLE hConsole;
    CONSOLE_FONT_INFOEX fontInfo;
    int cp = GetConsoleOutputCP();
    
    /* Set console control handler */
    SetConsoleCtrlHandler(ConsoleEventHandler, TRUE);
    
    /* Get the console output handle */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }
    
    /* Configure console for UTF-8 output */
    if (cp != CP_UTF8)
    {
        SetConsoleOutputCP(CP_UTF8);
        printf("Changed console code page from %d to %d (UTF-8)\n", cp, CP_UTF8);
    }
    
    /* Set console font to one with good Arabic support */
    fontInfo.cbSize = sizeof(CONSOLE_FONT_INFOEX);
    GetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);
    
    /* Try to use a font with good Arabic support */
    wcscpy(fontInfo.FaceName, L"Consolas");
    fontInfo.dwFontSize.X = 0;
    fontInfo.dwFontSize.Y = 16;
    fontInfo.FontFamily = FF_DONTCARE;
    fontInfo.FontWeight = FW_NORMAL;
    
    /* Set the new font */
    if (!SetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo))
    {
        printf("Failed to set console font. Arabic characters may not display correctly.\n");
        result = FALSE;
    }
    
    /* Configure console window visibility for GUI mode */
    if (g_GuiMode && !force_visible)
    {
        /* Hide the console window in GUI mode unless forced visible */
        ShowWindow(GetConsoleWindow(), SW_HIDE);
    }
    else
    {
        /* Show the console window */
        ShowWindow(GetConsoleWindow(), SW_SHOW);
    }
    
    return result;
}

/**
 * WindowProc - Window procedure for the main window
 * @hWnd: Window handle
 * @uMsg: Message
 * @wParam: Message parameter
 * @lParam: Message parameter
 *
 * Return: Result code
 */
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
        {
            /* Create the status bar */
            g_hStatusBar = create_status_bar(hWnd);
            
            /* Create the toolbar */
            g_hToolBar = create_toolbar(hWnd);
            
            /* Set default font for controls */
            if (g_hFont)
            {
                SendMessage(g_hStatusBar, WM_SETFONT, (WPARAM)g_hFont, TRUE);
                SendMessage(g_hToolBar, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            }
            
            /* Create console container */
            g_hConsoleContainer = CreateWindowEx(
                0,
                "STATIC",
                NULL,
                WS_CHILD | WS_VISIBLE | SS_SUNKEN,
                0, 0, 0, 0,
                hWnd,
                (HMENU)IDC_CONSOLE_CONTAINER,
                GetModuleHandle(NULL),
                NULL);
            
            return 0;
        }
        
        case WM_SIZE:
        {
            RECT rcClient, rcStatus, rcTool;
            int statusHeight, toolHeight, clientWidth, clientHeight;
            
            /* Get client area dimensions */
            GetClientRect(hWnd, &rcClient);
            clientWidth = rcClient.right - rcClient.left;
            clientHeight = rcClient.bottom - rcClient.top;
            
            /* Size and position the status bar */
            SendMessage(g_hStatusBar, WM_SIZE, 0, 0);
            GetWindowRect(g_hStatusBar, &rcStatus);
            statusHeight = rcStatus.bottom - rcStatus.top;
            
            /* Size and position the toolbar */
            SendMessage(g_hToolBar, TB_AUTOSIZE, 0, 0);
            GetWindowRect(g_hToolBar, &rcTool);
            toolHeight = rcTool.bottom - rcTool.top;
            
            /* Size and position the console container */
            MoveWindow(g_hConsoleContainer,
                     0, toolHeight,
                     clientWidth, clientHeight - statusHeight - toolHeight,
                     TRUE);
            
            return 0;
        }
        
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case ID_FILE_EXIT:
                    /* Exit application */
                    DestroyWindow(hWnd);
                    return 0;
                
                case ID_EDIT_CLEAR:
                    /* Clear console */
                    system("cls");
                    return 0;
                
                case ID_VIEW_FONT:
                    /* Change font - Not implemented yet */
                    MessageBox(hWnd, "Font selection not implemented yet", "Information", MB_OK | MB_ICONINFORMATION);
                    return 0;
                
                case ID_HELP_ABOUT:
                    /* Show about dialog */
                    MessageBox(hWnd, "Simple Shell\nModern UI with Arabic Support", "About", MB_OK | MB_ICONINFORMATION);
                    return 0;
            }
            break;
        }
        
        case WM_CTLCOLORSTATIC:
        {
            /* Set background color for console container */
            if ((HWND)lParam == g_hConsoleContainer && g_hBackgroundBrush)
            {
                SetBkMode((HDC)wParam, TRANSPARENT);
                SetTextColor((HDC)wParam, RGB(220, 220, 220)); /* Light gray text */
                return (INT_PTR)g_hBackgroundBrush;
            }
            break;
        }
        
        case WM_DESTROY:
        {
            /* Clean up and exit */
            PostQuitMessage(0);
            return 0;
        }
    }
    
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/**
 * convert_args - Convert wide character arguments to UTF-8
 * @argc: Argument count
 * @wargv: Wide character arguments
 *
 * Return: UTF-8 arguments
 */
char **convert_args(int argc, LPWSTR *wargv)
{
    int i;
    char **argv;
    
    /* Allocate memory for arguments */
    argv = (char **)malloc(sizeof(char *) * (argc + 1));
    if (!argv)
    {
        return NULL;
    }
    
    /* Convert each argument to UTF-8 */
    for (i = 0; i < argc; i++)
    {
        /* Get required buffer size */
        int size = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, NULL, 0, NULL, NULL);
        if (size <= 0)
        {
            /* Error converting argument */
            int j;
            for (j = 0; j < i; j++)
            {
                free(argv[j]);
            }
            free(argv);
            return NULL;
        }
        
        /* Allocate buffer for converted argument */
        argv[i] = (char *)malloc(size);
        if (!argv[i])
        {
            /* Error allocating memory */
            int j;
            for (j = 0; j < i; j++)
            {
                free(argv[j]);
            }
            free(argv);
            return NULL;
        }
        
        /* Convert argument to UTF-8 */
        WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, argv[i], size, NULL, NULL);
    }
    
    /* NULL-terminate argument array */
    argv[argc] = NULL;
    
    return argv;
}

/**
 * free_args - Free converted arguments
 * @argc: Argument count
 * @argv: Arguments to free
 */
void free_args(int argc, char **argv)
{
    int i;
    
    if (!argv)
    {
        return;
    }
    
    /* Free each argument */
    for (i = 0; i < argc; i++)
    {
        free(argv[i]);
    }
    
    /* Free argument array */
    free(argv);
}

/**
 * ShellThreadProc - Thread procedure for running the shell
 * @lpParameter: Thread parameter (argv)
 *
 * Return: Exit code
 */
DWORD WINAPI ShellThreadProc(LPVOID lpParameter)
{
    char **argv = (char **)lpParameter;
    int result = 0;
    
    /* Check if we got valid arguments */
    if (argv == (void *)1)
    {
        /* No arguments available, just run the shell */
        char *default_args[] = {"hsh", NULL};
        result = shell_main(1, default_args);
    }
    else if (argv)
    {
        /* Run the shell with the provided arguments */
        result = shell_main(0, argv);
    }
    else
    {
        /* No arguments provided, run with default */
        char *default_args[] = {"hsh", NULL};
        result = shell_main(1, default_args);
    }
    
    return result;
}

/**
 * initialize_console_for_gui - Initialize console for GUI mode
 *
 * Return: Result code
 */
int initialize_console_for_gui(void)
{
    FILE *fout, *fin, *ferr;
    int result = 0;
    MSG msg;
    LPWSTR *wargv;
    int argc;
    char **argv = NULL;
    
    /* Allocate a console for the application */
    if (!AllocConsole())
    {
        MessageBox(NULL, "Failed to allocate console!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }
    
    /* Open console streams */
    fout = freopen("CONOUT$", "w", stdout);
    fin = freopen("CONIN$", "r", stdin);
    ferr = freopen("CONOUT$", "w", stderr);
    
    if (!fout || !fin || !ferr)
    {
        MessageBox(NULL, "Failed to redirect console streams!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }
    
    *stdout = *fout;
    *stdin = *fin;
    *stderr = *ferr;
    
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    
    /* Parse command line into argc/argv */
    wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (wargv)
    {
        /* Convert wide character arguments to UTF-8 */
        argv = convert_args(argc, wargv);
        
        /* Create a thread to run the shell */
        HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ShellThreadProc, 
                                     argv ? argv : (void *)1, 0, NULL);
        if (!hThread)
        {
            printf("Failed to create shell thread\n");
            result = -1;
        }
        else
        {
            /* Process messages while the shell thread is running */
            DWORD threadStatus = STILL_ACTIVE;
            
            do {
                /* Check if there are messages to process */
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                    /* If it's a quit message, exit the message loop */
                    if (msg.message == WM_QUIT) {
                        /* Signal the shell thread to terminate */
                        TerminateThread(hThread, 0);
                        break;
                    }
                    
                    /* Process the message */
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
                
                /* Check if the thread is still running */
                GetExitCodeThread(hThread, &threadStatus);
                
                /* Give other threads a chance to run */
                Sleep(10);
                
            } while (threadStatus == STILL_ACTIVE);
            
            /* Get the thread's exit code */
            GetExitCodeThread(hThread, (LPDWORD)&result);
            CloseHandle(hThread);
        }
        
        /* Free the converted arguments */
        if (argv)
            free_args(argc, argv);
        
        /* Free the wide character arguments */
        LocalFree(wargv);
    }
    else
    {
        /* Message loop - simplified version if we can't get command line arguments */
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        result = (int)msg.wParam;
    }
    
    /* Display exit message */
    printf("\n\033[0m"); /* Reset any previous formatting */
    printf("\033[38;2;255;255;50m"); /* RGB color similar to Windows Terminal accent */
    printf("╔════════════════════════════════════════════════════╗\n");
    printf("║                                                    ║\n");
    printf("║              SIMPLE SHELL - MODERN UI              ║\n");
    printf("║         WITH ARABIC AND BAA LANGUAGE SUPPORT       ║\n");
    printf("║                                                    ║\n");
    printf("╚════════════════════════════════════════════════════╝\n");
    printf("\033[0m\n"); /* Reset text formatting */
    
    /* Test Arabic output */
    printf("\033[38;2;255;200;50m"); /* Gold color for Arabic text */
    printf("مرحبًا بكم في الصدفة البسيطة - واجهة مستخدم حديثة\n");
    printf("\033[0m\n"); /* Reset text formatting */
    
    /* Set the default console color scheme to match Windows Terminal */
    printf("\033[40m"); /* Black background */
    printf("\033[37;1m"); /* Bright white text */
    
    /* Display exit message */
    printf("\n\033[38;2;255;255;50mShell exited with code %d\033[0m\n", result);
    printf("\033[38;2;200;200;200mPress Enter to exit...\033[0m");
    fflush(stdout);
    getchar();
    
    /* Close file streams */
    fclose(fout);
    fclose(fin);
    fclose(ferr);
    
    /* Free the console */
    FreeConsole();
    
    return result;
}

/**
 * WinMain - Windows GUI entry point
 * @hInstance: The instance handle
 * @hPrevInstance: The previous instance handle (always NULL)
 * @lpCmdLine: Command line arguments
 * @nCmdShow: Show command
 *
 * Return: Exit code
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;
    HWND hWnd;
    WNDCLASSEX wc;
    HICON hIcon;
    
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);
    
    /* Initialize console handles */
    g_hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);
    g_hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
    
    /* Set GUI mode flag */
    g_GuiMode = TRUE;
    
    /* Load shell icon */
    hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SHELL_ICON));
    if (!hIcon)
    {
        /* Fallback to default application icon */
        hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }
    
    /* Create background brush */
    g_hBackgroundBrush = CreateSolidBrush(RGB(12, 12, 12)); /* Dark background similar to Windows Terminal */
    
    /* Create font */
    g_hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                         CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
    
    /* Initialize common controls */
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_WIN95_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);
    
    /* Register the window class */
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WindowProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = hIcon;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = g_hBackgroundBrush;
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "SimpleShellWindow";
    wc.hIconSm       = hIcon;
    
    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window registration failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }
    
    /* Create the main window */
    hWnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "SimpleShellWindow",
        "Simple Shell - Modern UI with Arabic Support",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL);
    
    if (!hWnd)
    {
        MessageBox(NULL, "Window creation failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }
    
    /* Initialize COM for the UI thread */
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr))
    {
        MessageBox(NULL, "COM initialization failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }
    
    /* Store main window handle */
    g_hMainWindow = hWnd;
    
    /* Show the window */
    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
    
    /* Configure console for Arabic text */
    setup_console_window(TRUE);
    
    /* Run the shell in a background thread */
    HANDLE hShellThread = CreateThread(NULL, 0, ShellThreadProc, NULL, 0, NULL);
    if (!hShellThread)
    {
        MessageBox(NULL, "Failed to start shell thread!", "Error", MB_ICONEXCLAMATION | MB_OK);
    }
    
    /* Message loop */
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    /* Wait for shell thread to finish */
    if (hShellThread)
    {
        WaitForSingleObject(hShellThread, INFINITE);
        CloseHandle(hShellThread);
    }
    
    /* Uninitialize COM */
    CoUninitialize();
    
    /* Clean up resources */
    if (g_hFont)
    {
        DeleteObject(g_hFont);
    }
    
    if (g_hBackgroundBrush)
    {
        DeleteObject(g_hBackgroundBrush);
    }
    
    return (int)msg.wParam;
}

#endif /* WINDOWS */

/**
 * main - Standard entry point for the application
 * @argc: Argument count
 * @argv: Argument array
 *
 * Return: Exit code
 */
int main(int argc, char *argv[])
{
#ifdef WINDOWS
    /* Check if we should run in GUI mode */
    g_GuiMode = FALSE;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--gui") == 0)
        {
            g_GuiMode = TRUE;
            break;
        }
    }
    
    /* If not in GUI mode, set up console and run in console mode */
    if (!g_GuiMode)
    {
        /* Set up console window with proper settings */
        setup_console_window(TRUE);
        
        /* Display startup message with a distinctive appearance */
        printf("\033[0m"); /* Reset any previous formatting */
        printf("\033[38;2;50;255;255m"); /* RGB color similar to Windows Terminal accent */
        printf("╔════════════════════════════════════════════════════╗\n");
        printf("║                                                    ║\n");
        printf("║              SIMPLE SHELL - CONSOLE MODE           ║\n");
        printf("║         WITH ARABIC AND BAA LANGUAGE SUPPORT       ║\n");
        printf("║                                                    ║\n");
        printf("╚════════════════════════════════════════════════════╝\n");
        printf("\033[0m\n"); /* Reset text formatting */
        
        /* Test Arabic output */
        printf("\033[38;2;255;200;50m"); /* Gold color for Arabic text */
        printf("مرحبًا بكم في الصدفة البسيطة - وضع وحدة التحكم\n");
        printf("\033[0m\n"); /* Reset text formatting */
        
#ifdef WINDOWS
        /* Parse command line into argc/argv */
        LPWSTR *wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (wargv)
        {
            /* Run the shell in console mode */
            int result = shell_main(argc, argv);
            LocalFree(wargv);
            return result;
        }
#endif
        
        return shell_main(argc, argv);
    }
    
    /* GUI mode initialization */
    return initialize_console_for_gui();
#else
    /* On non-Windows platforms, just run in console mode */
    return shell_main(argc, argv);
#endif
} 