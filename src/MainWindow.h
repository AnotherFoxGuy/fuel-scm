#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QStringList>
#include <QMap>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QSet>
#include "SettingsDialog.h"
#include "Bridge.h"

namespace Ui {
	class MainWindow;
}

//////////////////////////////////////////////////////////////////////////
// RepoFile
//////////////////////////////////////////////////////////////////////////
struct RepoFile
{
	enum EntryType
	{
		TYPE_UNKNOWN		= 1<<0,
		TYPE_UNCHANGED		= 1<<1,
		TYPE_EDITTED		= 1<<2,
		TYPE_ADDED			= 1<<3,
		TYPE_DELETED		= 1<<4,
		TYPE_MISSING		= 1<<5,
		TYPE_RENAMED		= 1<<6,
		TYPE_CONFLICTED		= 1<<7,
		TYPE_MODIFIED		= TYPE_EDITTED|TYPE_ADDED|TYPE_DELETED|TYPE_MISSING|TYPE_RENAMED|TYPE_CONFLICTED,
		TYPE_REPO			= TYPE_UNCHANGED|TYPE_MODIFIED,
		TYPE_ALL			= TYPE_UNKNOWN|TYPE_REPO
	};

	RepoFile(QFileInfo &info, EntryType type, const QString &repoPath)
	{
		FileInfo = info;
		Type = type;
		FilePath = getRelativeFilename(repoPath);
		Path = FileInfo.absolutePath();

		// Strip the workspace path from the path
		Q_ASSERT(Path.indexOf(repoPath)==0);
		Path = Path.mid(repoPath.length()+1);
	}

	bool isType(EntryType t) const
	{
		return Type == t;
	}

	void setType(EntryType t)
	{
		Type = t;
	}

	EntryType getType() const
	{
		return Type;
	}

	QFileInfo getFileInfo() const
	{
		return FileInfo;
	}

	const QString &getFilePath() const
	{
		return FilePath;
	}

	QString getFilename() const
	{
		return FileInfo.fileName();
	}

	const QString &getPath() const
	{
		return Path;
	}

	QString getRelativeFilename(const QString &path)
	{
		QString abs_base_dir = QDir(path).absolutePath();

		QString relative = FileInfo.absoluteFilePath();
		int index = relative.indexOf(abs_base_dir);
		if(index<0)
			return QString("");

		return relative.right(relative.length() - abs_base_dir.length()-1);
	}

private:
	QFileInfo	FileInfo;
	EntryType	Type;
	QString		FilePath;
	QString		Path;
};


typedef QSet<QString> stringset_t;

class Workspace
{
public:
	~Workspace();

	typedef QList<RepoFile*> filelist_t;
	typedef QMap<QString, RepoFile*> filemap_t;

	void				clearState();

	Bridge &			fossil() { return bridge; }
	const Bridge &		fossil() const { return bridge; }

	static bool			scanDirectory(QFileInfoList &entries, const QString& dirPath, const QString &baseDir, const QString ignoreSpec, const bool& abort, Bridge::UICallback &uiCallback);
	void				scanWorkspace(bool scanLocal, bool scanIgnored, bool scanModified, bool scanUnchanged, const QString &ignoreGlob, Bridge::UICallback &uiCallback, bool &operationAborted);

	QStandardItemModel	&getFileModel() { return repoFileModel; }
	QStandardItemModel	&getDirModel() { return repoDirModel; }
	QStandardItemModel	&getStashModel() { return repoStashModel; }
	filemap_t			&getFiles() { return workspaceFiles; }
	stringset_t			&getPaths() { return pathSet; }
	stashmap_t			&getStashes() { return stashMap; }

private:
	Bridge				bridge;
	filemap_t			workspaceFiles;
	stringset_t			pathSet;
	stashmap_t			stashMap;

	QStandardItemModel	repoFileModel;
	QStandardItemModel	repoDirModel;
	QStandardItemModel	repoStashModel;
};

//////////////////////////////////////////////////////////////////////////
// MainWindow
//////////////////////////////////////////////////////////////////////////
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(Settings &_settings, QWidget *parent = 0, QString *workspacePath = 0);
	~MainWindow();
	bool diffFile(const QString& repoFile);
	void fullRefresh();

private:
	bool refresh();
	void scanWorkspace();
	void applySettings();
	void updateSettings();
	const QString &getCurrentWorkspace();
	void setCurrentWorkspace(const QString &workspace);
	void log(const QString &text, bool isHTML=false);
	void setStatus(const QString &text);
	bool uiRunning() const;
	void getSelectionFilenames(QStringList &filenames, int includeMask=RepoFile::TYPE_ALL, bool allIfEmpty=false);
	void getFileViewSelection(QStringList &filenames, int includeMask=RepoFile::TYPE_ALL, bool allIfEmpty=false);
	void getDirViewSelection(QStringList &filenames, int includeMask=RepoFile::TYPE_ALL, bool allIfEmpty=false);
	void getStashViewSelection(QStringList &stashNames, bool allIfEmpty=false);
	void getSelectionPaths(stringset_t &paths);
	void getAllFilenames(QStringList &filenames, int includeMask=RepoFile::TYPE_ALL);
	bool startUI();
	void stopUI();
	void enableActions(bool on);
	void addWorkspace(const QString &dir);
	void rebuildRecent();
	bool openWorkspace(const QString &path);
	void loadFossilSettings();
	QString getFossilHttpAddress();
	void updateDirView();
	void updateFileView();
	void updateStashView();
	void selectRootDir();
	void fossilBrowse(const QString &fossilUrl);
	void dragEnterEvent(class QDragEnterEvent *event);
	void dropEvent(class QDropEvent *event);
	void setBusy(bool busy);
	virtual QMenu *createPopupMenu();

	enum ViewMode
	{
		VIEWMODE_LIST,
		VIEWMODE_TREE
	};

private slots:
	// Manual slots.
	// Use a different naming scheme to prevent warnings from Qt's automatic signaling
	void onOpenRecent();
	void onTreeViewSelectionChanged(const class QItemSelection &selected, const class QItemSelection &deselected);
	void onFileViewDragOut();
	void onAbort();

	// Designer slots
	void on_actionRefresh_triggered();
	void on_actionDiff_triggered();
	void on_actionFossilUI_triggered();
	void on_actionQuit_triggered();
	void on_actionTimeline_triggered();
	void on_actionHistory_triggered();
	void on_actionClearLog_triggered();
	void on_tableView_doubleClicked(const QModelIndex &index);
	void on_treeView_doubleClicked(const QModelIndex &index);
	void on_actionOpenFile_triggered();
	void on_actionPush_triggered();
	void on_actionPull_triggered();
	void on_actionCommit_triggered();
	void on_actionAdd_triggered();
	void on_actionDelete_triggered();
	void on_actionRevert_triggered();
	void on_actionOpenContaining_triggered();
	void on_actionRename_triggered();
	void on_actionUndo_triggered();
	void on_actionAbout_triggered();
	void on_actionUpdate_triggered();
	void on_actionSettings_triggered();
	void on_actionViewUnchanged_triggered();
	void on_actionViewModified_triggered();
	void on_actionViewUnknown_triggered();
	void on_actionViewIgnored_triggered();
	void on_actionViewAsList_triggered();
	void on_actionOpenFolder_triggered();
	void on_actionRenameFolder_triggered();
	void on_actionNewRepository_triggered();
	void on_actionOpenRepository_triggered();
	void on_actionCloseRepository_triggered();
	void on_actionCloneRepository_triggered();
	void on_actionViewStash_triggered();
	void on_actionNewStash_triggered();
	void on_actionApplyStash_triggered();
	void on_actionDeleteStash_triggered();
	void on_actionDiffStash_triggered();
	void on_textBrowser_customContextMenuRequested(const QPoint &pos);
	void on_tableView_customContextMenuRequested(const QPoint &pos);

private:
	class MainWinUICallback : public Bridge::UICallback
	{
	public:
		MainWinUICallback() : mainWindow(0)
		{}

		void init(class MainWindow *mainWindow)
		{
			this->mainWindow = mainWindow;
		}

		virtual void logText(const QString& text, bool isHTML);
		virtual void beginProcess(const QString& text);
		virtual void updateProcess(const QString& text);
		virtual void endProcess();
		virtual QMessageBox::StandardButton Query(const QString &title, const QString &query, QMessageBox::StandardButtons buttons);


	private:
		class MainWindow *mainWindow;
	};

	friend class MainWinUICallback;

	enum
	{
		MAX_RECENT=5
	};

	Ui::MainWindow		*ui;
	class QAction		*recentWorkspaceActs[MAX_RECENT];
	class QProgressBar	*progressBar;
	class QShortcut		*abortShortcut;
	bool				operationAborted;
	stringset_t			selectedDirs;	// The directory selected in the tree

	Workspace			workspace;
	Workspace &			getWorkspace() { return workspace; }

	Bridge &			fossil() { return workspace.fossil(); }
	const Bridge &		fossil() const { return workspace.fossil(); }

	Settings			&settings;
	QStringList			workspaceHistory;

	MainWinUICallback	uiCallback;

	ViewMode			viewMode;
};

#endif // MAINWINDOW_H

