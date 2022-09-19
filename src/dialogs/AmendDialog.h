#include <QDialog>

class QLineEdit;
class QTextEdit;

class AmendDialog: public QDialog {
public:
	AmendDialog(const QString &author, const QString &committer, const QString &commitMessage, QWidget *parent = nullptr);
	QString author() const;
	QString committer() const;
	QString commitMessage() const;

private:
	QLineEdit* m_author;
	QLineEdit* m_committer;
	QTextEdit* m_commitMessage;
};
