#include "dialogueeditor.h"

DialogueEditor::DialogueEditor(QWidget *parent) : QWidget(parent)
{
    initView();
    initStyle();

    setBucket(QList<DialogueBucket*>{}, nullptr);
}

void DialogueEditor::initView()
{
    avatar_btn = new QPushButton(this);
    name_label = new QLabel("名字", this);
    name_edit = new QLineEdit(this);
    said_label = new QLabel("说的话/旁白", this);
    said_edit = new QPlainTextEdit(this);
    style_label = new QLabel("样式表", this);
    style_edit = new QSSHighlightEditor(this);
    name_check = new QCheckBox("显示名字", this);
    delete_bucket_button = new QPushButton("删除此行", this);
    export_picture_button = new QPushButton("导出对话", this);
    save_figure_button = new QPushButton("保存角色", this);

    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->addWidget(avatar_btn);
    {
        circle_btn = new QPushButton("圆形头像", this);
        rounded_btn = new QPushButton("圆角头像", this);
        QHBoxLayout* avatar_hlayout = new QHBoxLayout;
        avatar_hlayout->addWidget(circle_btn);
        avatar_hlayout->addWidget(rounded_btn);
        vlayout->addLayout(avatar_hlayout);
    }
    vlayout->addWidget(name_label);
    vlayout->addWidget(name_edit);
    vlayout->addWidget(said_label);
    vlayout->addWidget(said_edit);
    vlayout->addWidget(style_label);
    vlayout->addWidget(style_edit);
    vlayout->addWidget(name_check);
    QHBoxLayout *button_hlayout = new QHBoxLayout;
    button_hlayout->addWidget(save_figure_button);
    button_hlayout->addWidget(delete_bucket_button);
    button_hlayout->addWidget(export_picture_button);
    vlayout->addLayout(button_hlayout);

    vlayout->setMargin(0);

    connect(avatar_btn, &QPushButton::clicked, this, [=]{
        if (!current_bucket || _loading_bucket)
            return ;
        QSettings st(QDir(data_dir).absoluteFilePath("settings.ini"), QSettings::IniFormat, this);
        QString r = st.value("recent/avatar_path", "").toString();

        QString path = QFileDialog::getOpenFileName(this, "选择头像", r, tr("Images (*.png *.xpm *.jpg)"));
        if (path.isEmpty())
            return ;
        st.setValue("recent/avatar_path", path);
        QPixmap pixmap(path);
        avatar_btn->setIconSize(getAvatarSize(pixmap.size()));
        avatar_btn->setIcon(pixmap);
        foreach (auto bucket, selected_buckets)
            bucket->setAvatar(pixmap);
    });
    connect(circle_btn, &QPushButton::clicked, this, [=]{
        if (!current_bucket || _loading_bucket)
            return ;
        QSettings st(QDir(data_dir).absoluteFilePath("settings.ini"), QSettings::IniFormat, this);
        QString r = st.value("recent/avatar_path", "").toString();

        QString path = QFileDialog::getOpenFileName(this, "选择头像", r, tr("Images (*.png *.xpm *.jpg)"));
        if (path.isEmpty())
            return ;
        st.setValue("recent/avatar_path", path);
        QPixmap pixmap(path);
        // 转成圆形
        {
            int radius = qMin(pixmap.width(), pixmap.height()) / 2;
            QPoint center(pixmap.width()/2, pixmap.height()/2);
            QRect usedRect(center.x()-radius, center.y()-radius, radius*2, radius*2);

            // 新图片
            QPixmap newPixmap(radius*2, radius*2);
            newPixmap.fill(Qt::transparent);
            QPainter painter(&newPixmap);
            QPainterPath path;
            path.addEllipse(0, 0, radius*2, radius*2);
            painter.setClipPath(path, Qt::IntersectClip);
            painter.drawPixmap(QRect(0,0,radius*2,radius*2), pixmap, usedRect);
            pixmap = newPixmap;
        }
        avatar_btn->setIconSize(getAvatarSize(pixmap.size()));
        avatar_btn->setIcon(pixmap);
        foreach (auto bucket, selected_buckets)
            bucket->setAvatar(pixmap);
    });
    connect(rounded_btn, &QPushButton::clicked, this, [=]{
        if (!current_bucket || _loading_bucket)
            return ;
        QSettings st(QDir(data_dir).absoluteFilePath("settings.ini"), QSettings::IniFormat, this);
        QString r = st.value("recent/avatar_path", "").toString();

        QString path = QFileDialog::getOpenFileName(this, "选择头像", r, tr("Images (*.png *.xpm *.jpg)"));
        if (path.isEmpty())
            return ;
        st.setValue("recent/avatar_path", path);
        QPixmap pixmap(path);
        // 转成圆角
        {
            int radius = qMin(pixmap.width(), pixmap.height()) / 2;
            QPoint center(pixmap.width()/2, pixmap.height()/2);
            QRect usedRect(center.x()-radius, center.y()-radius, radius*2, radius*2);

            // 新图片
            QPixmap newPixmap(radius*2, radius*2);
            newPixmap.fill(Qt::transparent);
            QPainter painter(&newPixmap);
            QPainterPath path;
            int cor = radius / 6;
            path.addRoundedRect(0, 0, radius*2, radius*2, cor, cor);
            painter.setClipPath(path, Qt::IntersectClip);
            painter.drawPixmap(QRect(0,0,radius*2,radius*2), pixmap, usedRect);
            pixmap = newPixmap;
        }
        avatar_btn->setIconSize(getAvatarSize(pixmap.size()));
        avatar_btn->setIcon(pixmap);
        foreach (auto bucket, selected_buckets)
            bucket->setAvatar(pixmap);
    });
    connect(name_edit, &QLineEdit::textEdited, this, [=]{
        if (!current_bucket || _loading_bucket)
            return ;
        foreach (auto bucket, selected_buckets)
            bucket->nickname->setText(name_edit->text());
    });
    connect(said_edit, &QPlainTextEdit::textChanged, this, [=]{
        if (!current_bucket || _loading_bucket)
            return ;

        foreach (auto bucket, selected_buckets)
        {
            QLabel* label = bucket->isNarrator() ? (QLabel*)bucket->narrator : (QLabel*)bucket->bubble;
            label->setText(said_edit->toPlainText());
            label->adjustSize();
            emit bucket->signalBubbleChanged();
        }
    });
    connect(style_edit, &QPlainTextEdit::textChanged, this, [=]{
        if (!current_bucket || _loading_bucket)
            return ;
        foreach (auto bucket, selected_buckets)
            bucket->setStyleSheet(style_edit->toPlainText());
    });
    connect(name_check, &QCheckBox::stateChanged, this, [=](int){
        if (!current_bucket || _loading_bucket)
            return ;
        foreach (auto bucket, selected_buckets)
            bucket->setNameVisible(name_check->isChecked());
    });

    connect(delete_bucket_button, &QPushButton::clicked, this, [=]{
        emit signalDelete();
    });
    connect(save_figure_button, &QPushButton::clicked, this, [=]{
        foreach (auto bucket, selected_buckets)
            emit signalSaveFigure(bucket);
    });
    connect(export_picture_button, &QPushButton::clicked, this, [=]{
        emit signalSaveToFile();
    });
}

void DialogueEditor::initStyle()
{
}

void DialogueEditor::setBucket(QList<DialogueBucket *> buckets, DialogueBucket *bucket)
{
    _loading_bucket = true;
    selected_buckets = buckets;
    current_bucket = bucket;
    if (bucket == nullptr)
    {
        avatar_btn->setEnabled(false);
        circle_btn->setEnabled(false);
        rounded_btn->setEnabled(false);
        name_label->setEnabled(false);
        name_edit->setEnabled(false);
        said_label->setEnabled(false);
        said_edit->setEnabled(false);
        style_label->setEnabled(false);
        style_edit->setEnabled(false);
        name_check->setEnabled(false);
        save_figure_button->setEnabled(false);
        delete_bucket_button->setEnabled(false);
        return ;
    }

    style_edit->setPlainText(bucket->styleSheet());
    if (bucket->isNarrator())
    {
        avatar_btn->setEnabled(false);
        circle_btn->setEnabled(false);
        rounded_btn->setEnabled(false);
        name_label->setEnabled(false);
        name_edit->setEnabled(false);
        said_label->setEnabled(true);
        said_edit->setEnabled(true);
        style_label->setEnabled(true);
        style_edit->setEnabled(true);
        name_check->setEnabled(false);
        save_figure_button->setEnabled(true);
        delete_bucket_button->setEnabled(false);

        said_edit->setPlainText(bucket->narrator->text());
    }
    else
    {
        avatar_btn->setEnabled(true);
        circle_btn->setEnabled(true);
        rounded_btn->setEnabled(true);
        name_label->setEnabled(true);
        name_edit->setEnabled(true);
        said_label->setEnabled(true);
        said_edit->setEnabled(true);
        style_label->setEnabled(true);
        style_edit->setEnabled(true);
        name_check->setEnabled(true);
        save_figure_button->setEnabled(true);
        delete_bucket_button->setEnabled(true);

        QPixmap pixmap = *bucket->avatar->pixmap();
        avatar_btn->setIcon(QIcon(pixmap));
        avatar_btn->setIconSize(getAvatarSize(pixmap.size()));
        name_edit->setText(bucket->nickname->text());
        said_edit->setPlainText(bucket->bubble->text());
    }
    _loading_bucket = false;
}

void DialogueEditor::setDataDir(QString dir)
{
    this->data_dir = dir;
}

QSize DialogueEditor::getAvatarSize(QSize size)
{
    return QSize(
                qMin(64, size.width()),
                qMin(64, size.height())
                );
}

void DialogueEditor::focusSaid()
{
    QTimer::singleShot(100, [=]{
        said_edit->setFocus();
        said_edit->selectAll();
    });
}
