using System;
using System.Diagnostics;
using System.Windows.Forms;

namespace GoA_AIO
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        const string WAITING_STRING = "Waiting for KINGDOM HEARTS II FINAL MIX";
        const string ATTACHED_STRING = "Garden of Assemblage injected";

        private void SetGameAttachedLabels(bool toggle)
        {
            if (toggle)
            {
                gameStatusLabel.Text = ATTACHED_STRING;
                notifyIcon1.Text = ATTACHED_STRING;
            }
            else
            {
                gameStatusLabel.Text = WAITING_STRING;
                notifyIcon1.Text = WAITING_STRING;
            }
        }
        
        private void checkTimer_Tick(object sender, EventArgs e)
        {
            Process[] processes = Process.GetProcessesByName("KINGDOM HEARTS II FINAL MIX");
            if (processes.Length == 0)
            {
                // TODO(skettios): set application to wait for process
                SetGameAttachedLabels(false);

                checkTimer.Stop();
                waitTimer.Start();
            }
        }

        private void waitTimer_Tick(object sender, EventArgs e)
        {
            SetGameAttachedLabels(false);

            Process[] processes = Process.GetProcessesByName("KINGDOM HEARTS II FINAL MIX");
            if (processes.Length > 0)
            {
                var process = processes[0];
                if (true) // TODO(skettios): inject backend to process
                {
                    SetGameAttachedLabels(true);

                    waitTimer.Stop();
                    checkTimer.Start();
                }
            }
        }

        private void Form1_Resize(object sender, EventArgs e)
        {
            if (WindowState == FormWindowState.Minimized)
            {
                Hide();
            }
        }

        private void notifyIcon1_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            Show();
            WindowState = FormWindowState.Normal;
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            checkTimer.Tick += checkTimer_Tick;
            waitTimer.Tick += waitTimer_Tick;
            waitTimer.Start();

            trayExit.Click += trayExit_Click;
        }

        private void trayExit_Click(object sender, EventArgs e)
        {
            Close();
        }
    }
}
