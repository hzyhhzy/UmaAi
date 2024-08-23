using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.IO;
using System.Threading.Tasks;
using System.Windows.Forms;
//using static System.Windows.Forms.VisualStyles.VisualStyleElement;
using Newtonsoft.Json.Linq;
using System.Xml.Linq;
using static System.Windows.Forms.VisualStyles.VisualStyleElement.Button;
using System.Runtime.Remoting.Messaging;
using System.Text.RegularExpressions;

namespace UmaAiForm
{
    public partial class Form1 : Form
    {
        private JObject umaDB, cardDB;

        public Form1()
        {
            InitializeComponent();
            RefreshTrainingComboBoxItems();
            ClearTrainingComboBoxSelection();
        }
        private void InitTextboxes()
        {
            numericUpDown34_ValueChanged(numericUpDown34, EventArgs.Empty);
            numericUpDown27_ValueChanged(numericUpDown27, EventArgs.Empty);
            numericUpDown14_ValueChanged(numericUpDown14, EventArgs.Empty);
            numericUpDown15_ValueChanged(numericUpDown15, EventArgs.Empty);
            numericUpDown16_ValueChanged(numericUpDown16, EventArgs.Empty);
            numericUpDown17_ValueChanged(numericUpDown17, EventArgs.Empty);
            numericUpDown18_ValueChanged(numericUpDown18, EventArgs.Empty);
            numericUpDown19_ValueChanged(numericUpDown19, EventArgs.Empty);
        }
        private void LoadJsonData()
        {
            string umaDbPath = "./db/umaDB.json";
            string cardDbPath = "./db/cardDB.json";
            try
            {
                if (File.Exists(umaDbPath))
                {
                    string jsonData = File.ReadAllText(umaDbPath);
                    umaDB = JObject.Parse(jsonData);
                }
                else
                {
                    umaDB = new JObject();
                }

                if (File.Exists(cardDbPath))
                {
                    string jsonData = File.ReadAllText(cardDbPath);
                    cardDB = JObject.Parse(jsonData);
                    textBox4.Text = jsonData;
                }
                else
                {
                    cardDB = new JObject();
                }
            }
            catch (Exception)
            {
                umaDB = new JObject();
                cardDB = new JObject();
            }
        }
        private string getCardName(int id)
        {
            if (cardDB != null && cardDB[id.ToString()] != null && cardDB[id.ToString()]["cardName"] != null)
            {
                return cardDB[id.ToString()]["cardName"].ToString();
            }
            else
                return "Unknown Card";
        }
        private string getUmaName(int id)
        {
            if (umaDB != null && umaDB[id.ToString()] != null && umaDB[id.ToString()]["name"] != null)
            {
                return umaDB[id.ToString()]["name"].ToString();
            }
            else
                return "Unknown Uma";
        }

        private void Form1_Load(object sender, EventArgs e)
        {

            LoadJsonData();
            InitTextboxes();
        }

        private int HarvestLoopCount(int turn)
        {
            bool isXiahesuOrUra = (turn >= 36 && turn <= 39) || (turn >= 60 && turn <= 63) || turn >= 72;
            return isXiahesuOrUra ? 0 : turn % 4;
        }

        private void tableLayoutPanel1_Paint(object sender, PaintEventArgs e)
        {

        }

        private void label8_Click(object sender, EventArgs e)
        {

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
            textBox11.Text = $"Year {year}, Month {month}, {halfMonth}";
            //harvest history
            int enabledHistNum = HarvestLoopCount(turn);
            comboBox27.Enabled = enabledHistNum > 0;
            comboBox28.Enabled = enabledHistNum > 1;
            comboBox29.Enabled = enabledHistNum > 2;
            comboBox30.Enabled = enabledHistNum > 3;
            checkBox1.Enabled = enabledHistNum > 0;
            checkBox2.Enabled = enabledHistNum > 1;
            checkBox3.Enabled = enabledHistNum > 2;
            checkBox4.Enabled = enabledHistNum > 3;
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
        /*
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
        }*/


        private void label48_Click(object sender, EventArgs e)
        {

        }

        private void numericUpDown14_ValueChanged(object sender, EventArgs e)
        {
            textBox4.Text = getCardName((int)numericUpDown14.Value); 
            RefreshTrainingComboBoxItems();
        }

        private void numericUpDown15_ValueChanged(object sender, EventArgs e)
        {
            textBox5.Text = getCardName((int)numericUpDown15.Value);
            RefreshTrainingComboBoxItems();
        }


        private void numericUpDown16_ValueChanged(object sender, EventArgs e)
        {

            textBox6.Text = getCardName((int)numericUpDown16.Value);
            RefreshTrainingComboBoxItems();

        }
        private void numericUpDown17_ValueChanged(object sender, EventArgs e)
        {
            textBox7.Text = getCardName((int)numericUpDown17.Value);
            RefreshTrainingComboBoxItems();
        }

        private void numericUpDown18_ValueChanged(object sender, EventArgs e)
        {
            textBox8.Text = getCardName((int)numericUpDown18.Value);
            RefreshTrainingComboBoxItems();
        }

        private void numericUpDown19_ValueChanged(object sender, EventArgs e)
        {
            textBox9.Text = getCardName((int)numericUpDown19.Value);
            RefreshTrainingComboBoxItems();
        }

        private void textBox4_TextChanged(object sender, EventArgs e)
        {

        }

        private void numericUpDown27_ValueChanged(object sender, EventArgs e)
        {
            textBox10.Text = getUmaName((int)numericUpDown27.Value);

        }


        private void label52_Click(object sender, EventArgs e)
        {

        }
        private int reviseOver1200(int x)
        {
            return x <= 1200 ? x : 1200 + 2 * (x - 1200);
        }
        private JObject stageToJson()
        {
            JObject stage = new JObject();
            stage["umaId"] = (int)numericUpDown27.Value;
            stage["umaStar"] = (int)numericUpDown13.Value;
            stage["islegal"] = true;
            int turn = (int)numericUpDown34.Value;
            stage["turn"] = turn;
            stage["vital"] = (int)numericUpDown11.Value;
            stage["maxVital"] = (int)numericUpDown12.Value;
            stage["motivation"] = (int)comboBoxMotivation.SelectedIndex + 1;
            if((int)stage["motivation"]<=0)
            {
                throw new Exception("Motivation not set!");
            }
            stage["fiveStatus"] = new JArray {
                reviseOver1200((int)numericUpDown1.Value),
                reviseOver1200((int)numericUpDown2.Value),
                reviseOver1200((int)numericUpDown3.Value),
                reviseOver1200((int)numericUpDown4.Value),
                reviseOver1200((int)numericUpDown5.Value) };
            stage["fiveStatusLimit"] = new JArray {
                reviseOver1200((int)numericUpDown6.Value),
                reviseOver1200((int)numericUpDown7.Value),
                reviseOver1200((int)numericUpDown8.Value),
                reviseOver1200((int)numericUpDown9.Value),
                reviseOver1200((int)numericUpDown10.Value) };
            stage["ptScoreRate"] = (double)numericUpDown51.Value;
            stage["skillPt"] = (int)((int)numericUpDown50.Value + (int)numericUpDown26.Value / ((double)stage["ptScoreRate"]));
            stage["skillScore"] = 0;
            stage["failureRateBias"] = (checkedListBox1.GetItemChecked(2) ? -2 : 0) + (checkedListBox1.GetItemChecked(3) ? 2 : 0);
            stage["isAiJiao"] = checkedListBox1.GetItemChecked(1);
            stage["isQieZhe"] = checkedListBox1.GetItemChecked(0);
            stage["isPositiveThinking"] = checkedListBox1.GetItemChecked(4);
            stage["zhongMaBlueCount"] = new JArray {
                (int)numericUpDown56.Value,
                (int)numericUpDown57.Value,
                (int)numericUpDown58.Value,
                (int)numericUpDown59.Value,
                (int)numericUpDown60.Value
            };
            //stage["saihou"] = 35;//not used
            stage["isRacing"] = false;

            var cardIds = new int[] {
                (int)numericUpDown14.Value,
                (int)numericUpDown15.Value,
                (int)numericUpDown16.Value,
                (int)numericUpDown17.Value,
                (int)numericUpDown18.Value,
                (int)numericUpDown19.Value
            };
            var cardLvs = new int[] {
                (int)numericUpDown20.Value,
                (int)numericUpDown21.Value,
                (int)numericUpDown22.Value,
                (int)numericUpDown23.Value,
                (int)numericUpDown24.Value,
                (int)numericUpDown25.Value
            };
            var friendships = new int[] {
                (int)numericUpDown62.Value,
                (int)numericUpDown63.Value,
                (int)numericUpDown64.Value,
                (int)numericUpDown65.Value,
                (int)numericUpDown66.Value,
                (int)numericUpDown67.Value,
                (int)numericUpDown68.Value,
                (int)numericUpDown69.Value
            };
            var isHints = new bool[] {
                checkBox5.Checked,
                checkBox6.Checked,
                checkBox7.Checked,
                checkBox8.Checked,
                checkBox9.Checked,
                checkBox10.Checked
            };



            JArray persons = new JArray();
            int lianghuaType = 0; //0 no lianghua, 1 SSR, 2 R
            int lianghuaIndex = 0; //index of lianghua card
            for (int i = 0; i < 6; i++)//cards
            {
                JObject person = new JObject();
                person["isCard"] = true;
                int cardID = cardIds[i];
                int cardBreakNum = (cardLvs[i] / 5 - 6) + 3 - cardID / 10000;
                if (cardBreakNum < 0) cardBreakNum = 0;
                if (cardBreakNum > 4) cardBreakNum = 4;
                person["cardID"] = cardID * 10 + cardBreakNum;

                if (cardID == 10104)
                {
                    lianghuaType = 2;
                    lianghuaIndex = i;
                }
                if (cardID == 30188)
                {
                    lianghuaType = 1;
                    lianghuaIndex = i;
                }
                bool isLianghua = cardID == 30188 || cardID == 10104;
                person["personType"] = isLianghua ? 1 : 2;
                person["friendship"] = friendships[i];
                person["isHint"] = isHints[i];
                person["cardRecord"] = 0;//not used
                person["friendOrGroupCardStage"] = 0;//not used
                person["groupCardShiningContinuousTurns"] = 0;//not used

                persons.Add(person);
            }
            //yayoi,journalist
            {
                JObject person = new JObject();
                person["isCard"] = false;
                person["cardID"] = 0;
                person["personType"] = 4;
                person["friendship"] = friendships[6];
                person["isHint"] = false;
                person["cardRecord"] = 0;//not used
                person["friendOrGroupCardStage"] = 0;//not used
                person["groupCardShiningContinuousTurns"] = 0;//not used

                persons.Add(person);
            }
            {
                JObject person = new JObject();
                person["isCard"] = false;
                person["cardID"] = 0;
                person["personType"] = 5;
                person["friendship"] = friendships[7];
                person["isHint"] = false;
                person["cardRecord"] = 0;//not used
                person["friendOrGroupCardStage"] = 0;//not used
                person["groupCardShiningContinuousTurns"] = 0;//not used

                persons.Add(person);
            }
            //non-card lianghua
            {
                JObject person = new JObject();
                person["isCard"] = false;
                person["cardID"] = 0;
                person["personType"] = lianghuaType == 0 ? 0 : 6;
                person["friendship"] = 0;//not accurate, but not important
                person["isHint"] = false;
                person["cardRecord"] = 0;//not used
                person["friendOrGroupCardStage"] = 0;//not used
                person["groupCardShiningContinuousTurns"] = 0;//not used

                persons.Add(person);
            }




            stage["cardId"] = new JArray {
                persons[0]["cardID"],
                persons[1]["cardID"],
                persons[2]["cardID"],
                persons[3]["cardID"],
                persons[4]["cardID"],
                persons[5]["cardID"]
                };

            stage["persons"] = persons;

            var personDistributionArray = new bool[,]
            {

            };

            JArray personDistribution = new JArray();


            for(int i = 0; i < 5; i++)
            {
                var heads = new int[] { -1, -1, -1, -1, -1 };
                int headNum = 0;
                for (int j = 0; j < 8; j++)
                {
                    if (personDistributionArray[j, i])
                    {
                        if(headNum >= 5)
                        {
                            throw new Exception("More than five persons in one training");
                        }
                        heads[headNum] = j;
                        headNum++;
                    }
                }
                personDistribution.Add(new JArray(heads));
            }

            stage["personDistribution"] = personDistribution;

            stage["lockedTrainingId"] = -1;//not used



            int[] trainLevels=new int[] {
            };
            stage["uaf_trainingLevel"] = new JArray(trainLevels);

            int uafFinished = turn < 24 ? 0 : turn < 36 ? 1 : turn < 48 ? 2 : turn < 60 ? 3 : turn < 72 ? 4 : 5;
            bool[] uaf_winHistory = Enumerable.Repeat(false, 75).ToArray();
            for (int i = 0; i < uafFinished * 15; i++)
                uaf_winHistory[i] = true;
            //have lose?
            var havelose = new bool[]
            {
            };
            if (uafFinished > 0)
            {
                for (int c = 0; c < 3; c++)
                {
                    if (!havelose[c]) continue;
                    //set the lowest level training as a lose 
                    int loseIdx = Enumerable.Range(c * 5, 5)
                         .OrderBy(index => stage["uaf_trainingLevel"][index])
                         .First();
                    uaf_winHistory[(uafFinished - 1) * 15 + loseIdx] = false;

                }
            }


            int[] uaf_levelColor_total = new int[]
            {
                trainLevels.Skip(0).Take(5).Sum(),
                trainLevels.Skip(5).Take(5).Sum(),
                trainLevels.Skip(10).Take(5).Sum()
            };
            stage["uaf_buffActivated"] = new JArray
            {
                uaf_levelColor_total[0]/50,
                uaf_levelColor_total[1]/50,
                uaf_levelColor_total[2]/50
            };
            stage["uaf_buffNum"] = new JArray {
            };

            stage["lianghua_type"] = lianghuaType;
            stage["lianghua_personId"] = lianghuaIndex;
            stage["lianghua_outgoingStage"] =
                lianghuaType == 0 ? 0 :
                comboBoxFriendStage.SelectedIndex == 1 ? 0 :
                comboBoxFriendStage.SelectedIndex == 2 ? 1 :
                comboBoxFriendStage.SelectedIndex == 3 ? 2 :
                -1;
            if((int)stage["lianghua_outgoingStage"]==-1)
                throw new Exception("Friend card stage is empty or wrong");
            stage["lianghua_outgoingUsed"] = lianghuaType == 0 ? 0 : (int)numericUpDown61.Value;

            return stage;
        }

        private void button3_Click(object sender, EventArgs e)
        {
            saveFile("./thisTurn.json");
        }


        private void radioButton4_CheckedChanged(object sender, EventArgs e)
        {
            
        }

        private void radioButton11_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void checkBox6_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void RefreshTrainingComboBoxItems()
        {
            // 获取所有的TextBox内容
            string[] cardNames = new string[6];
            cardNames[0] = textBox4.Text;
            cardNames[1] = textBox5.Text;
            cardNames[2] = textBox6.Text;
            cardNames[3] = textBox7.Text;
            cardNames[4] = textBox8.Text;
            cardNames[5] = textBox9.Text;

            string[] comboBoxItems = new string[10];
            comboBoxItems[0] = "None";
            for (int i = 0; i < 6; i++)
            {
                comboBoxItems[i + 1] = "C" + (i + 1) + ":" + cardNames[i];
            }
            comboBoxItems[7] = "Yayoi(Non-card)";
            comboBoxItems[8] = "Reporter";
            comboBoxItems[9] = "NPC";

            // 设置每个ComboBox的选项
            foreach (Control control in tableLayoutPanel4.Controls)
            {
                if (control is ComboBox comboBox)
                {
                    int idx =int.Parse(Regex.Match(control.Name, @"\d+").Value);
                    comboBox.Items.Clear();
                    comboBox.Items.AddRange(comboBoxItems);
                }
            }
        }

        private void ClearTrainingComboBoxSelection()
        {
            // 设置每个ComboBox的选项
            foreach (Control control in tableLayoutPanel4.Controls)
            {
                if (control is ComboBox comboBox)
                {
                    comboBox.SelectedIndex = 0;
                }
            }
        }
        private void numericUpDown61_ValueChanged(object sender, EventArgs e)
        {

        }

        private void numericUpDown28_ValueChanged(object sender, EventArgs e)
        {
            int lvcount = (int)numericUpDown28.Value;
            int lv = lvcount / 4 + 1;
            label36.Text = "Lv " + lv.ToString() + "  ,Count=";
        }

        private void label36_Click(object sender, EventArgs e)
        {

        }

        private void numericUpDown29_ValueChanged(object sender, EventArgs e)
        {
            int lvcount = (int)numericUpDown29.Value;
            int lv = lvcount / 4 + 1;
            label40.Text = "Lv " + lv.ToString() + "  ,Count=";
        }

        private void numericUpDown30_ValueChanged(object sender, EventArgs e)
        {
            int lvcount = (int)numericUpDown30.Value;
            int lv = lvcount / 4 + 1;
            label41.Text = "Lv " + lv.ToString() + "  ,Count=";
        }

        private void numericUpDown31_ValueChanged(object sender, EventArgs e)
        {
            int lvcount = (int)numericUpDown31.Value;
            int lv = lvcount / 4 + 1;
            label43.Text = "Lv " + lv.ToString() + "  ,Count=";
        }

        private void numericUpDown32_ValueChanged(object sender, EventArgs e)
        {
            int lvcount = (int)numericUpDown32.Value;
            int lv = lvcount / 4 + 1;
            label44.Text = "Lv " + lv.ToString() + "  ,Count=";
        }

        private void button4_Click(object sender, EventArgs e)
        {
            ClearTrainingComboBoxSelection();
        }

        private void comboBox26_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void numericUpDown6_ValueChanged(object sender, EventArgs e)
        {

        }

        private void label51_Click(object sender, EventArgs e)
        {

        }

        private void numericUpDown49_ValueChanged(object sender, EventArgs e)
        {
            if(numericUpDown49.Value >= 12000)
            {
                checkBox11.Checked = true;
                checkBox11.Enabled = false;
            }
            else
            {
                checkBox11.Checked = false;
                checkBox11.Enabled = true;
            }
        }

        private void label59_Click(object sender, EventArgs e)
        {

        }

        private void label60_Click(object sender, EventArgs e)
        {

        }

        private void saveFile(string fileName)
        {
            try
            {
                JObject j = stageToJson();
                string jsonString = j.ToString();

                // 将字符串保存到指定文件路径
                File.WriteAllText(fileName, jsonString);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error saving JSON data to file: " + ex.Message);
            }

        }
    }
}
