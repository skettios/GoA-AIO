using System;
using System.IO;
using System.Text;
using System.Runtime.InteropServices;

namespace GoA
{
    class Native
    {
        [DllImport("kernel32.dll")]
        public static extern IntPtr LoadLibrary(string dll);

        [DllImport("kernel32.dll")]
        public static extern bool FreeLibrary(IntPtr handle);

        [DllImport("kernel32.dll")]
        public static extern IntPtr OpenProcess(int access, bool inherit, int procId);

        [DllImport("kernel32.dll", CharSet = CharSet.Auto)]
        public static extern IntPtr GetModuleHandle(string name);

        [DllImport("kernel32.dll", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
        public static extern IntPtr GetProcAddress(IntPtr module, string procName);

        [DllImport("kernel32.dll", SetLastError = true, ExactSpelling = true)]
        public static extern IntPtr VirtualAllocEx(IntPtr proc, IntPtr funcAddress, uint size, uint allocType, uint protect);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool WriteProcessMemory(IntPtr proc, IntPtr baseAddress, byte[] buffer, uint size, out UIntPtr numBytesWrote);

        [DllImport("kernel32.dll")]
        public static extern IntPtr CreateRemoteThread(IntPtr proc, IntPtr threadAttribs, uint stackSize, IntPtr startAddress, IntPtr param, uint creationFlags, IntPtr threadId);

        [DllImport("kernel32", CharSet = CharSet.Auto, SetLastError = true)]
        public static extern bool CloseHandle(IntPtr handle);

        public const int PROCESS_CREATE_THREAD = 0x0002;
        public const int PROCESS_QUERY_INFORMATION = 0x0400;
        public const int PROCESS_VM_OPERATION = 0x0008;
        public const int PROCESS_VM_WRITE = 0x0020;
        public const int PROCESS_VM_READ = 0x0010;

        public const int PROCESS_ALL_ACCESS = PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ;

        public const uint MEM_COMMIT = 0x00001000;
        public const uint MEM_RESERVE = 0x00002000;
        public const uint PAGE_READWRITE = 4;

        public const uint MEM_ALL = MEM_COMMIT | MEM_RESERVE;
    }

    public class Backend
    {
        private IntPtr KH2Handle;
        private IntPtr DLLHandle;
        private string DLLPath;

        public Backend()
        {
            KH2Handle = IntPtr.Zero;
            DLLHandle = IntPtr.Zero;
            DLLPath = Directory.GetCurrentDirectory() + "\\GoA-Backend.dll";
        }

        public void Reset()
        {
            Native.CloseHandle(KH2Handle);
            Native.FreeLibrary(DLLHandle);

            KH2Handle = IntPtr.Zero;
            DLLHandle = IntPtr.Zero;
        }

        public bool Inject(int procId)
        {
            if (KH2Handle != IntPtr.Zero)
                return false;

            uint dllPathSize = (uint)((DLLPath.Length + 1) * Marshal.SizeOf<char>());

            KH2Handle = Native.OpenProcess(Native.PROCESS_ALL_ACCESS, false, procId);
            if (KH2Handle == IntPtr.Zero)
                return false;

            IntPtr loadLibraryAddress = Native.GetProcAddress(Native.GetModuleHandle("kernel32.dll"), "LoadLibraryA");
            if (loadLibraryAddress == IntPtr.Zero)
                return false;

            IntPtr dllPathAddress = Native.VirtualAllocEx(KH2Handle, IntPtr.Zero, dllPathSize, Native.MEM_ALL, Native.PAGE_READWRITE);
            if (dllPathAddress == IntPtr.Zero)
                return false;

            UIntPtr bytesWritten;
            if (!Native.WriteProcessMemory(KH2Handle, dllPathAddress, Encoding.Default.GetBytes(DLLPath), dllPathSize, out bytesWritten))
                return false;

            IntPtr threadHandle = Native.CreateRemoteThread(KH2Handle, IntPtr.Zero, 0, loadLibraryAddress, dllPathAddress, 0, IntPtr.Zero);
            if (threadHandle == IntPtr.Zero)
                return false;
            else
            {
                Native.CloseHandle(threadHandle);
                Native.CloseHandle(loadLibraryAddress);
            }

            return true;
        }

        struct BackendConfig
        {
            public IntPtr scriptsDirectory;
            public IntPtr proccessExecutablePath;
            public byte keybindsEnabled;
            public byte running;
        }

        public void Run(string executablePath)
        {
            DLLHandle = Native.LoadLibrary("GoA-Backend.dll");

            var config = new BackendConfig()
            {
                keybindsEnabled = 0,
                running = 1
            };

            string scriptsDir = Directory.GetCurrentDirectory() + "\\scripts";
            IntPtr scriptsDirAddress = Native.VirtualAllocEx(KH2Handle, IntPtr.Zero, (uint)(scriptsDir.Length * Marshal.SizeOf<char>()), Native.MEM_ALL, Native.PAGE_READWRITE);
            UIntPtr bytesWritten;
            Native.WriteProcessMemory(KH2Handle, scriptsDirAddress, Encoding.Default.GetBytes(scriptsDir), (uint)(scriptsDir.Length + Marshal.SizeOf<char>()), out bytesWritten);

            IntPtr executablePathAddress = Native.VirtualAllocEx(KH2Handle, IntPtr.Zero, (uint)(executablePath.Length * Marshal.SizeOf<char>()), Native.MEM_ALL, Native.PAGE_READWRITE);
            Native.WriteProcessMemory(KH2Handle, executablePathAddress, Encoding.Default.GetBytes(executablePath), (uint)(executablePath.Length * Marshal.SizeOf<char>()), out bytesWritten);

            config.scriptsDirectory = scriptsDirAddress;
            config.proccessExecutablePath = executablePathAddress;

            byte[] bytes = new byte[Marshal.SizeOf<BackendConfig>()];
            IntPtr ptr = Marshal.AllocHGlobal(Marshal.SizeOf<BackendConfig>());
            Marshal.StructureToPtr(config, ptr, true);
            Marshal.Copy(ptr, bytes, 0, Marshal.SizeOf<BackendConfig>());
            Marshal.FreeHGlobal(ptr);

            IntPtr configAddress = Native.VirtualAllocEx(KH2Handle, IntPtr.Zero, (uint)Marshal.SizeOf<BackendConfig>(), Native.MEM_ALL, Native.PAGE_READWRITE);
            Native.WriteProcessMemory(KH2Handle, configAddress, bytes, (uint)Marshal.SizeOf<BackendConfig>(), out bytesWritten);

            IntPtr runAddress = Native.GetProcAddress(DLLHandle, "GoA_Run");
            IntPtr threadHandle = Native.CreateRemoteThread(KH2Handle, IntPtr.Zero, 0, runAddress, configAddress, 0, IntPtr.Zero);
            if (threadHandle != IntPtr.Zero)
                Native.CloseHandle(threadHandle);
        }
    }
}
