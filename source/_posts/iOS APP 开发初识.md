---
title: iOS APP 开发初识
tags:
  - iOS
categories:
  - 编程
date: 2018-06-26 02:25:37
---

1. 创建项目
2. 在 storyboard 中构建 UI ，Auto Layout 和 stack view 是重点
3. IBoutlet 是代码到 UI ，IBAction 是 UI 到代码，比如你想控制一个 button 的按下的动作，那么你需要为 button 定义它的 IBAction ，然后在按钮按下后，可能会触发其他变化，比如你在文本框输入了文本，按下提交，将你输入的文本替换到页面的一个 Label 上，这个时候你就需要去修改 IBoutlet 的变量。
4. viewDidLoad -> viewWillAppear -> viewDidAppear -> viewWillDisappear -> viewDidDisappear
<!-- more -->
![life_circle](http://od3b21nvv.bkt.clouddn.com/hexoimg750169d3cdc7c657fb359e8c1492a72e.png)

5. delegation：

   1. 在不同的 UI 对应的不同  viewcontroller 中去声明代理，首先

   ```
   class ViewController : UIViewController, AVAudioRecorderDelegate {
     code
     ...
     audioRecorder.delegate = self
   }
   ```

   2. 其实调用相应 Delegate 中触发代理的函数。然后配合 segue，这里留到下一点说。

6. 多个视图间通过segue通信

```
performSegue(withIdentifier: "StopRecording", sender: audioRecorder.url) //代理完成了某项工作后，要想把数据传向下一个 VC 的话，就需要performSegue，withIdentifier是指storyboard中的segue的id。

//在performSegue后需要为下一个VC接受数据做准备，而prepare一般是当前的VC来定义，所以一般是前一个VC中定义。这里需要先定义好下一个VC和里面的相关变量，如这里的playSoundsVC.recordedAudioURL。
override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if segue.identifier == "StopRecording" {
            let playSoundsVC = segue.destination as! PlaySoundsViewController
            let recordedAudioURL = sender as! URL
            playSoundsVC.recordedAudioURL = recordedAudioURL
        }
    }
//自此，就完成了数据在两个VC中的传递
```

7. 然后是 extension ，可以为每个类在额外的文件中添加代码，意义就是组件化吧，现在看来，格式就是

```
extension PlaySoundsViewController: AVAudioPlayerDelegate {
}
```

8. 还有一些 swift 的语法还是有点模糊，等我上完 udacity 的另外一门课再开一篇总结。


