using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Microsoft.VisualBasic.FileIO;
using System.Runtime.InteropServices;

namespace QuotesChart
{
    public partial class FormMirandaQuotesChart : Form
    {
        public FormMirandaQuotesChart()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            string[] cmd_line_args = Environment.GetCommandLineArgs();

            for (int i = 1; i < cmd_line_args.Length; ++i)
            {
                string data_file = cmd_line_args[i];
                AddDataFromFile(data_file);
            }
        }

        private void dateFrom_ValueChanged(object sender, EventArgs e)
        {
            if (dateFrom.Value < dateTo.Value)
            {
                chartQuotes.ChartAreas[0].AxisX.Minimum = dateFrom.Value.ToOADate();
                chartQuotes.Invalidate();
            }
        }

        private void dateTo_ValueChanged(object sender, EventArgs e)
        {
            if (dateTo.Value > dateFrom.Value)
            {
                chartQuotes.ChartAreas[0].AxisX.Maximum = dateTo.Value.ToOADate();
                chartQuotes.Invalidate();
            }
        }

        int WM_COPYDATA = 0x4A;

        public struct COPYDATASTRUCT
        {
            public IntPtr dwData;
            public int cbData;
            [MarshalAs(UnmanagedType.LPWStr)]
            public String lpData;
        }

        protected override void WndProc(ref Message msg)
        {
            if (msg.Msg == WM_COPYDATA)
            {
                COPYDATASTRUCT cp = (COPYDATASTRUCT)Marshal.PtrToStructure(msg.LParam, typeof(COPYDATASTRUCT));

                if (/**(cp.dwData) == 0x1945 && */cp.lpData != null)
                {
                    AddDataFromFile(cp.lpData);
                }

            }
            base.WndProc(ref msg);
        }

        private void AddDataFromFile(string data_file)
        {
            using (TextFieldParser parser = new Microsoft.VisualBasic.FileIO.TextFieldParser(data_file))
            {
                parser.TrimWhiteSpace = true;
                parser.TextFieldType = FieldType.Delimited;
                parser.SetDelimiters("\t");

                System.Windows.Forms.DataVisualization.Charting.Series series = null;
                while (true)
                {
                    string[] parts = parser.ReadFields();
                    if (parts == null)
                    {
                        break;
                    }

                    if (parts.Length >= 3)
                    {
                        string name = parts[0];
                        string date = parts[1];
                        string value = parts[2];

                        if (series == null)
                        {
                            if (chartQuotes.Series.FindByName(name) == null)
                            {
                                chartQuotes.Series.Add(name);
                                series = chartQuotes.Series[name];
                                series.ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Line;
                            }
                            else
                            {
                                break;
                            }
                        }

                        DateTime dt;
                        double d;
                        if (Double.TryParse(value, out d) && DateTime.TryParse(date, out dt))
                        {
                            series.Points.AddXY(dt, d);

                            if (!dateMin.HasValue)
                            {
                                dateMin = new DateTime();
                                dateMin = dt;
                            }
                            else if (dt < dateMin)
                            {
                                dateMin = dt;
                            }

                            if (!dateMax.HasValue)
                            {
                                dateMax = new DateTime();
                                dateMax = dt;
                            }
                            else if (dt > dateMax)
                            {
                                dateMax = dt;
                            }
                        }
                    }
                }
            }

            if (dateMin.HasValue && dateMax.HasValue)
            {
                chartQuotes.ChartAreas[0].AxisX.Minimum = dateMin.Value.ToOADate();
                chartQuotes.ChartAreas[0].AxisX.Maximum = dateMax.Value.ToOADate();

                dateFrom.Value = dateMin.Value;
                dateTo.Value = dateMax.Value;
            }
        }

        private DateTime? dateMin = null;
        private DateTime? dateMax = null;
    }
}
