#include "AmendDialog.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QHBoxLayout>

AmendDialog::AmendDialog(const QString& author, const QString& committer, const QString& commitMessage, QWidget *parent): QDialog(parent) {

	auto* l = new QGridLayout();


	// committer
	// author
	// message

	auto* lCommitter = new QLabel(tr("Committer:"), this);
	m_committer = new QLineEdit(committer, this);
	l->addWidget(lCommitter,  0, 0);
	l->addWidget(m_committer,  0, 1);

	auto* lAuthor = new QLabel(tr("Author:"), this);
	m_author = new QLineEdit(author, this);
	l->addWidget(lAuthor,  1, 0);
	l->addWidget(m_author,  1, 1);

	auto* lMessage = new QLabel(tr("Message:"), this);
	m_commitMessage = new QTextEdit(commitMessage, this);
	l->addWidget(lMessage,  2, 0);
	l->addWidget(m_commitMessage,  2, 1);

	auto* ok = new QPushButton(tr("Amend"), this);
	auto* cancel = new QPushButton(tr("Cancel"), this);

	connect(ok, &QPushButton::clicked, this, &QDialog::accept);
	connect(cancel, &QPushButton::clicked, this, &QDialog::reject);

	auto* hl = new QHBoxLayout();
	hl->addWidget(cancel);
	hl->addWidget(ok);

	l->addLayout(hl, 3, 1);

	setLayout(l);
}

QString AmendDialog::author() const {
	return m_author->text();
}

QString AmendDialog::committer() const {
	return m_committer->text();
}

QString AmendDialog::commitMessage() const {
	return m_commitMessage->toPlainText();
}
