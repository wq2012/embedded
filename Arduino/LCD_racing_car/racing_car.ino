// Video demo: https://www.youtube.com/watch?v=AaIZ4X26AM8

#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int switchPin = 6;
int switchState = 0;
int prevSwitchState = 0;

long frameStepMs = 300;
long prevFrame = 0;
long totalFrame = 0;
bool gameRunning = false;

class Scene
{
public:
  Scene();
  void Render();
  void ShowStartMessage();
  void ShowDeathMessage();
  void Update();
  void MoveCar();
  void Restart();

private:
  char contents_[2][16];
  char car_ = '>';
  char obstacle_ = 'O';
};

Scene::Scene()
{
  gameRunning = false;
}

void Scene::ShowStartMessage()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ready?");
  lcd.setCursor(0, 1);
  lcd.print("Press to start");
  delay(10);
}

void Scene::Restart()
{
  for (int r = 0; r < 2; r++)
  {
    for (int c = 0; c < 16; c++)
    {
      contents_[r][c] = ' ';
    }
  }
  contents_[0][0] = car_;
  gameRunning = true;
  frameStepMs = 300;
  prevFrame = 0;
  totalFrame = 0;
}
void Scene::Render()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(contents_[0]);
  lcd.setCursor(0, 1);
  lcd.print(contents_[1]);
  delay(10);
}

void Scene::ShowDeathMessage()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Car crashed...");
  lcd.setCursor(0, 1);
  lcd.print("Press to restart");
  delay(10);
  gameRunning = false;
}

void Scene::Update()
{
  // Death check.
  for (int r = 0; r < 2; r++)
  {
    if (contents_[r][0] == car_ && contents_[r][1] != ' ')
    {
      ShowDeathMessage();
      return;
    }
  }
  // Left shift.
  for (int r = 0; r < 2; r++)
  {
    for (int c = 0; c < 15; c++)
    {
      if (contents_[r][c] != car_)
      {
        contents_[r][c] = contents_[r][c + 1];
      }
    }
  }
  // New column.
  int num = random(4);
  if (num == 0)
  {
    contents_[0][15] = obstacle_;
    contents_[1][15] = ' ';
  }
  else if (num == 1)
  {
    contents_[0][15] = ' ';
    contents_[1][15] = obstacle_;
  }
  else
  {
    contents_[0][15] = ' ';
    contents_[1][15] = ' ';
  }
  // No must-die scenarios.
  for (int r = 0; r < 2; r++)
  {
    if (contents_[r][14] != ' ')
    {
      contents_[1 - r][15] = ' ';
    }
  }
  Serial.print("\nRow 1: ");
  for (int i = 0; i < 16; i++)
    Serial.print(contents_[0][i]);
  Serial.print("\nRow 2: ");
  for (int i = 0; i < 16; i++)
    Serial.print(contents_[1][i]);
  Serial.println("");
}

void Scene::MoveCar()
{
  for (int r = 0; r < 2; r++)
  {
    if (contents_[r][0] == car_)
    {
      if (contents_[1 - r][0] == obstacle_)
      {
        ShowDeathMessage();
        return;
      }
      else
      {
        contents_[1 - r][0] = car_;
        contents_[r][0] = ' ';
        return;
      }
    }
  }
}

Scene scene;

void setup()
{
  lcd.begin(16, 2);
  Serial.begin(9600);
  pinMode(switchPin, INPUT);
  scene.ShowStartMessage();
}

void loop()
{
  // Handle switch.
  bool needToRender = false;
  switchState = digitalRead(switchPin);
  delay(10);
  if (switchState != prevSwitchState && switchState == HIGH)
  {
    if (!gameRunning)
    {
      scene.Restart();
      needToRender = true;
    }
    else
    {
      scene.MoveCar();
      needToRender = true;
    }
  }
  prevSwitchState = switchState;

  // Handle frame.
  long frame = millis();
  if (frame - prevFrame >= frameStepMs)
  {
    scene.Update();
    needToRender = true;
    totalFrame++;
    prevFrame = frame;
    if (totalFrame > 0 && totalFrame % 10 == 0)
    {
      if (frameStepMs > 200)
      {
        frameStepMs -= 20;
      }
    }
  }

  // Do rendering.
  if (gameRunning && needToRender)
  {
    scene.Render();
  }
}
