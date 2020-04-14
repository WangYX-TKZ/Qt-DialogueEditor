/**
  * 剧本 对话编辑器
  * 带 widgets、actions
  */

#ifndef DIALOGUEGROUP_H
#define DIALOGUEGROUP_H

#include <QObject>
#include <QWidget>
#include <QApplication>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QScrollBar>
#include <QMenu>
#include <QAction>
#include "dialoguebucket.h"
#include "dialogueeditor.h"
#include "dialoguemanager.h"

class DialogueGroup : public QWidget
{
    Q_OBJECT
public:
    DialogueGroup(QWidget *parent = nullptr);
    DialogueGroup(QString dir, QWidget *parent = nullptr);

    void initView();
    void initStyle();
    void initData();

    void setDataDirAndLoad(QString dir);

protected:
    void resizeEvent(QResizeEvent* event);

signals:

public slots:
    void slotAddLeftChat();
    void slotAddNarrator();
    void slotAddRightChat();
    void slotListMenuShowed(QPoint pos);
    void slotSaveFigure(DialogueBucket* bucket);
    void refreshFigures();

    void actionInsertLeftChat();
    void actionInsertNarrator();
    void actionInsertRightChat();
    void actionChatMoveUp();
    void actionChatMoveDown();
    void actionChatDelete();

private:
    QListWidgetItem *addChat(DialogueBucket* bucket, int row = -1);

private:
    QListWidget *dialogues_list_widget, *figure_list_widget;
    QPushButton *left_button, *mid_button, *right_button;
    DialogueEditor *editor;

    DialogueManager* manager;
    QList<DialogueBucket*> buckets;
};

#endif // DIALOGUEGROUP_H
