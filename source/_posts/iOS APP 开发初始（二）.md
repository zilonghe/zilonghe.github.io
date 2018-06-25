---
title: iOS APP 开发初始（二）
tags:
  - iOS
categories:
  - 编程
date: 2018-06-26 02:24:37
---

## ViewController 之间的三种通信方式
<!-- more -->
1. 纯代码实现

   ```swift
       @IBAction func playGameByCodeOnly(sender: UIButton) {
           let myChoice = sender.tag
           let computerChoice = getComputerChoice()

           var controller: GameResultViewController
           controller = self.storyboard?.instantiateViewController(withIdentifier: "GameResultViewController") as! GameResultViewController
           controller.result = getResult(myChoice, computerChoice: computerChoice)
           controller.resultImageName = getResultImageName(myChoice, computerChoice: computerChoice)
           present(controller, animated:true, completion:nil)
       }
   	
   ```

   - 首先定义@IBAction，将方法绑定到制定的控件（如 UIButton）
   - 通过`controller = self.storyboard?.instantiateViewController(withIdentifier: "GameResultViewController") as! GameResultViewController`获得下一个 VC 的controller对象
   - 为下一个 VC 指定各种数据，达到两个 VC 间通信的效果
   - 最后`present(controller, animated:true, completion:nil)`

2. 代码结合 segue

   ```swift
       @IBAction func playGameBySegueAndCode(sender: UIButton) {
           performSegue(withIdentifier: "GameOnSegueAndCode", sender: sender)
       }
       
       override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
           if segue.identifier == "GameOnSegueAndCode" {
               let GameResultViewController = segue.destination as! GameResultViewController
               let button = sender as! UIButton
               let myChoice = button.tag
               let computerChoice = getComputerChoice()
               GameResultViewController.result = getResult(myChoice, computerChoice: computerChoice)
               GameResultViewController.resultImageName = getResultImageName(myChoice, computerChoice: computerChoice)
           }
       }
   ```

   - 在storyboard中为两个 VC 直接定义 segue，为segue定义 identifier
   - 定义 @IBAction，并将方法绑定到指定控件
   - 先performSegue，再prepareforSegue
   - 关于sender是按钮，在prepareforSegue中访问这个按钮的问题：看[这里](http://stackoverflow.com/questions/28524642/swift-prepareforsegue-fails-to-pass-sender-tag-nil-value-in-other-viewcontrol)
     - 若在performSegue的时候传入sender是`self`的话，那么sender就是 VC 本身这个实例，此时你在prepare中想访问 VC 的 tag 属性的话，需要将sender对应地改成你想要访问的tag属性的对象的实例，比如UIButton，如上例中所示。

3. 纯segue

```swift
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if segue.identifier == "GameOnSegueOnly" {
            let GameResultViewController = segue.destination as! GameResultViewController
            let button = sender as! UIButton
            let myChoice = button.tag
            let computerChoice = getComputerChoice()
            GameResultViewController.result = getResult(myChoice, computerChoice: computerChoice)
            GameResultViewController.resultImageName = getResultImageName(myChoice, computerChoice: computerChoice)
        }
    }
```

- 在storyboard中直接在 当前 VC 中的控件（如按钮）与下一个 VC 定义 segue，为segue定义identifier
- 不需要定义@IBAction
- 不需要执行performSegue，直接prepareForSegue