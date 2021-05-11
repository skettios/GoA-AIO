using System;
using System.Diagnostics;
using System.Windows.Forms;
using System.IO;
using YamlDotNet.RepresentationModel;
using System.Runtime.InteropServices;

namespace GoA
{
    public partial class Form1 : Form
    {
        private Backend backend;
        private RandomizerConfig randofig;

        [DllImport("kernel32.dll")]
        static extern bool AllocConsole();

        public Form1()
        {
            InitializeComponent();
            backend = new Backend();
            randofig = new RandomizerConfig()
            {
                treasures = new ushort[592]
            };
            for (var i = 0; i < 592; i++)
                randofig.treasures[i] = 0x00;
        }

        const string WAITING_STRING = "Waiting for KINGDOM HEARTS II FINAL MIX";
        const string ATTACHED_STRING = "Garden of Assemblage Mod running";

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

                backend.Reset();

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
                if (backend.Inject(process.Id)) // TODO(skettios): inject backend to process
                {
                    SetGameAttachedLabels(true);

                    backend.Run(process.MainModule.ModuleName);

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
            trayKeybinds.CheckOnClick = true;

            checkTimer.Tick += checkTimer_Tick;
            waitTimer.Tick += waitTimer_Tick;
            
            trayExit.Click += trayExit_Click;
            trayKeybinds.CheckedChanged += trayKeybinds_OnCheckedChanged;

            waitTimer.Start();
            
            Hide();
        }

        private void trayExit_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void trayKeybinds_OnCheckedChanged(object sender, EventArgs e)
        {
        }

        private void traySeed_Click(object sender, EventArgs e)
        {
            using (var fileDialog = new OpenFileDialog())
            {
                fileDialog.InitialDirectory = Directory.GetCurrentDirectory();
                fileDialog.Filter = "zip files (*.zip)|*.zip|yaml files (*.yml)|*.yml|All files (*.*)|*.*";
                fileDialog.FilterIndex = 1;
                fileDialog.RestoreDirectory = true;

                if (fileDialog.ShowDialog() == DialogResult.OK)
                {
                    var filePath = fileDialog.FileName;
                    var fileStream = fileDialog.OpenFile();

                    using (StreamReader reader = new StreamReader(fileStream))
                    {
                        var yaml = new YamlStream();
                        yaml.Load(reader);

                        var mapping = (YamlMappingNode)yaml.Documents[0].RootNode;
                        foreach (var entry in mapping.Children)
                        {
                            uint location = uint.Parse(entry.Key.ToString());
                            ushort itemId = ushort.Parse(entry.Value["ItemId"].ToString());
                            randofig.treasures[location] = itemId;
                        }

                        reader.Close();
                    }
                }
            }
        }
    }
}
