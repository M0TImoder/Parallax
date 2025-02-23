///
/// @file parallax.hpp
/// @brief 並列処理用ライブラリ
/// 
/// このライブラリはmillis()を用いた並列処理用の関数を提供します。
///
/// @note 約50日経過するとmillis()がオーバーフローします。
/// @author MOTImoder
/// @version 1.0
///

//
// 基本動作
// setupProcess()で実行したいメンバ関数をマクロとして登録 -> execute()で実行
//

#pragma once

#include <Arduino.h>
#include <functional>

namespace parallax {

  /// @brief 並列処理するメンバ関数の基本単位を設定
  class Task {
  public:
    /// @brief Taskオブジェクトのコンストラクタ
    /// @param func 実際のメンバ関数のように実行される関数
    Task(const std::function<void(void)> &func) : function(func) {}

    /// @brief コンストラクタの中身を実行
    void operator()() {
      if (function) {
        function();
      }
    }
    
  private:
    std::function<void(void)> function;
  };

  /// @brief メンバ関数に名前を付けマクロとして登録
  /// @param name マクロの名前
  /// @param code マクロの中身のメンバ関数
  #define setupProcess(name, code) \
    parallax::Task name([](){ code; })

  /// @brief マクロを指定時間内実行するようスケジュール
  /// @param task マクロの名前
  /// @param duration 実行時間(ms)
  void execute(Task &task, unsigned long duration);

  /// @brief スケジュールされたマクロの実行状態を更新
  void update();

  /// @brief 実行を待機しているマクロのリンクリストのノード
  struct ScheduledTask {
    Task* task;               // 登録された処理へのポインタ
    unsigned long startTime;  // 実行開始時刻 millis()
    unsigned long duration;   // 実行期間 ms
    ScheduledTask *next;      // 次の処理へのポインタ
  };

  /// @brief 登録済みマクロ群の先頭ポインタ
  ScheduledTask *head = nullptr;

  /// @brief すべてのマクロを非同期で指定された時間の間だけ実行
  void execute(Task &task, unsigned long duration) {
    ScheduledTask *newTask = new ScheduledTask;

    newTask->task = &task;
    newTask->startTime = millis();
    newTask->duration = duration;
    newTask->next = nullptr;

    // マクロのリスト追加処理
    // 多分これが一番効率的

    // リストが空の場合
    if (head == nullptr) {
      head = newTask;
    } 

    // 1つ以上マクロが存在する場合
    else {
      newTask->next = head;
      head = newTask;
    }
  }

  /// @brief スケジュールされたマクロの実行状態を更新
  void update() {
    unsigned long currentTime = millis();
    // リンクリストを走査する際、前ノードのポインタ経由で操作する
    ScheduledTask **indirect = &head;

    // リスト内に存在する処理を全て実行する
    while (*indirect) {
      // 実行中の関数をcurrentに位置付ける
      ScheduledTask *current = *indirect;
      
      // 指定時間内か判定
      if (currentTime - current->startTime < current->duration) {
        // オーバーロードで関数実行
        (*(current->task))();
        indirect = &current->next;
      }

      // 実行期間を超えたタスクはリストから除去しメモリ解放
      else {
        *indirect = current->next;
        delete current;
      }
    }
  }

} // namespace parallax
