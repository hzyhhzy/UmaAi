using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;

namespace UmaAiForm
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }
        

        private void tableLayoutPanel1_Paint(object sender, PaintEventArgs e)
        {

        }

        private void label8_Click(object sender, EventArgs e)
        {

        }

        private void textBox1_KeyPress(object sender, KeyPressEventArgs e)
        {
            HandleKeyPress(e);
        }

        private void textBox2_KeyPress(object sender, KeyPressEventArgs e)
        {
            HandleKeyPress(e);
        }

        private void textBox3_KeyPress(object sender, KeyPressEventArgs e)
        {
            HandleKeyPress(e);
        }

        private void textBox4_KeyPress(object sender, KeyPressEventArgs e)
        {

            HandleKeyPress(e);
        }

        private void textBox5_KeyPress(object sender, KeyPressEventArgs e)
        {

            HandleKeyPress(e);
        }

        private void textBox6_KeyPress(object sender, KeyPressEventArgs e)
        {

            HandleKeyPress(e);
        }

        private void textBox7_KeyPress(object sender, KeyPressEventArgs e)
        {

            HandleKeyPress(e);
        }

        private void textBox8_KeyPress(object sender, KeyPressEventArgs e)
        {

            HandleKeyPress(e);
        }

        private void textBox9_KeyPress(object sender, KeyPressEventArgs e)
        {

            HandleKeyPress(e);
        }

        private void textBox10_KeyPress(object sender, KeyPressEventArgs e)
        {

            HandleKeyPress(e);
        }


        private void HandleKeyPress(KeyPressEventArgs e)
        {
            if (!char.IsControl(e.KeyChar) && !char.IsDigit(e.KeyChar))
            {
                e.Handled = true;
            }
        }

        private void ValidateInput(System.Windows.Forms.TextBox textBox, int maxValue)
        {
            if (int.TryParse(textBox.Text, out int value))
            {
                if (value >= maxValue)
                {
                    textBox.Text = maxValue.ToString(); // 设置为最大有效值
                    textBox.SelectionStart = textBox.Text.Length; // 光标移到最后
                }
            }
            else if (!string.IsNullOrEmpty(textBox.Text))
            {
                textBox.Text = string.Empty;
            }
        }
        

        private void label9_Click(object sender, EventArgs e)
        {

        }

        private void textBox11_TextChanged(object sender, EventArgs e)
        {
        }

        private void label10_Click(object sender, EventArgs e)
        {

        }

        private void textBox11_KeyPress(object sender, KeyPressEventArgs e)
        {
            HandleKeyPress(e);
        }

        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void label19_Click(object sender, EventArgs e)
        {

        }

        private void tableLayoutPanel2_Paint(object sender, PaintEventArgs e)
        {

        }

        private void numericUpDown34_ValueChanged(object sender, EventArgs e)
        {
            int turn = (int)numericUpDown34.Value;
            int year = 1 + turn / 24;
            int month = 1 + (turn % 24) / 2;
            string halfMonth = turn % 2 == 1 ? "latter half" : "first half";
            label27.Text=$"Year {year}, Month {month}, {halfMonth}";
        }

        private void label32_Click(object sender, EventArgs e)
        {

        }

        private void label28_Click(object sender, EventArgs e)
        {

        }

        private void tableLayoutPanel3_Paint(object sender, PaintEventArgs e)
        {

        }

        private void label34_Click(object sender, EventArgs e)
        {

        }
    }
}
