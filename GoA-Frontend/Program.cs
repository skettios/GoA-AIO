using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace GoA
{
    static class Program
    {
        /// <summary>
        ///  The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            bool result;
            var mutex = new System.Threading.Mutex(true, "GOAFRONTEND", out result);
            if (!result)
            {
                MessageBox.Show("GoA-Frontend already running!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }

            Application.SetHighDpiMode(HighDpiMode.SystemAware);
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Form1());

            GC.KeepAlive(mutex);
        }
    }
}
