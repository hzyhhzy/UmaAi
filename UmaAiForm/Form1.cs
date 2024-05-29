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

        private void label37_Click(object sender, EventArgs e)
        {

        }

        private void label38_Click(object sender, EventArgs e)
        {

        }

        private void checkedListBox1_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void tableLayoutPanel4_Paint(object sender, PaintEventArgs e)
        {

        }

        private void colorComboBox_DrawItem(System.Windows.Forms.ComboBox box, DrawItemEventArgs e)
        {

            // 如果没有项，直接返回
            if (e.Index < 0) return;

            // 获取当前项的文本
            string text = box.Items[e.Index].ToString();

            // 设置绘制背景颜色
            e.DrawBackground();

            // 根据项的文本设置不同的颜色
            Color color;
            Color fontColor = Color.Black;
            switch (text)
            {
                case "Red":
                    color = Color.Red;
                    fontColor = Color.Black;
                    break;
                case "Blue":
                    color = Color.Blue;
                    fontColor = Color.White;
                    break;
                case "Yellow":
                    color = Color.Yellow;
                    fontColor = Color.Black;
                    break;
                default:
                    color = e.BackColor;
                    break;
            }

            // 设置绘制背景颜色
            using (SolidBrush brush = new SolidBrush(color))
            {
                e.Graphics.FillRectangle(brush, e.Bounds);
            }

            // 绘制文本
            using (SolidBrush textBrush = new SolidBrush(fontColor))
            {
                e.Graphics.DrawString(text, e.Font, textBrush, e.Bounds);
            }

            // 绘制焦点框
            e.DrawFocusRectangle();
        }

        private void colorComboBox_SelectedIndexChanged(System.Windows.Forms.ComboBox box)
        {
            string text = box.Text.ToString();

            // 根据项的文本设置不同的颜色
            Color color;
            Color fontColor = Color.Black;
            switch (text)
            {
                case "Red":
                    color = Color.Red;
                    fontColor = Color.Black;
                    break;
                case "Blue":
                    color = Color.Blue;
                    fontColor = Color.White;
                    break;
                case "Yellow":
                    color = Color.Yellow;
                    fontColor = Color.Black;
                    break;
                default:
                    color = comboBox2.BackColor;
                    fontColor = comboBox2.ForeColor;
                    break;
            }
            box.BackColor = color;
            box.ForeColor = fontColor;
            box.Invalidate();
        }
        private void comboBox2_DrawItem(object sender, DrawItemEventArgs e)
        {
            colorComboBox_DrawItem(comboBox2, e);
        }

        private void comboBox2_SelectedIndexChanged(object sender, EventArgs e)
        {
            colorComboBox_SelectedIndexChanged(comboBox2);
        }

        private void comboBox3_SelectedIndexChanged(object sender, EventArgs e)
        {

            colorComboBox_SelectedIndexChanged(comboBox3);
        }

        private void comboBox3_DrawItem(object sender, DrawItemEventArgs e)
        {

            colorComboBox_DrawItem(comboBox3, e);
        }

        private void comboBox4_SelectedIndexChanged(object sender, EventArgs e)
        {
            colorComboBox_SelectedIndexChanged(comboBox4);

        }

        private void comboBox4_DrawItem(object sender, DrawItemEventArgs e)
        {
            colorComboBox_DrawItem(comboBox4, e);

        }

        private void comboBox5_SelectedIndexChanged(object sender, EventArgs e)
        {

            colorComboBox_SelectedIndexChanged(comboBox5);
        }

        private void comboBox5_DrawItem(object sender, DrawItemEventArgs e)
        {

            colorComboBox_DrawItem(comboBox5, e);
        }

        private void comboBox6_SelectedIndexChanged(object sender, EventArgs e)
        {

            colorComboBox_SelectedIndexChanged(comboBox6);
        }

        private void comboBox6_DrawItem(object sender, DrawItemEventArgs e)
        {

            colorComboBox_DrawItem(comboBox6, e);
        }

        private void label48_Click(object sender, EventArgs e)
        {

        }

        private void label52_Click(object sender, EventArgs e)
        {

        }
    }
}
