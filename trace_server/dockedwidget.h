#pragma  once

class DockedWidgetBase {

		void loadConfig (QString const & path);
		void saveConfig (QString const & path);

		QList<DecodedCommand> m_queue;
		void handleCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
		void commitCommands (E_ReceiveMode mode);

	// find
	void endOfSearch ();
	void findAllTexts (QString const & text);
	void findText (QString const & text, tlv::tag_t tag);
	void findText (QString const & text);
	void findNext (QString const & text);
	void findPrev (QString const & text);

	void scrollToCurrentTag ();
	void scrollToCurrentSelection ();
	void scrollToCurrentTagOrSelection ();
	void nextToView ();

	// filtering stuff
	void onInvalidateFilter ();
	//void syncSelection (QModelIndexList const & sel);

	//QAbstractTableModel const * modelView () const { return static_cast<QAbstractTableModel const *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : model()); }
	//QAbstractProxyModel const * proxyView () const { return static_cast<QAbstractProxyModel const *>(m_table_view_proxy); }

	//bool isModelProxy () const;

	//int findColumn4TagInTemplate (tlv::tag_t tag) const;
	//int findColumn4Tag (tlv::tag_t tag) const;
	//int appendColumn (tlv::tag_t tag);

	FilterWidget * filterWidget () { return m_config_ui.m_ui->widget; }
	FilterWidget const * filterWidget () const { return m_config_ui.m_ui->widget; }





	public slots:

		void onShow ();
		void onHide ();
		void onHideContextMenu ();
		void onShowContextMenu (QPoint const & pos);

		void setConfigValuesToUI (LogConfig const & cfg);
		void setUIValuesToConfig (LogConfig & cfg);

		void onSaveButton ();
		void onApplyButton ();
		void onResetButton ();
		void onDefaultButton ();
		void scrollTo (QModelIndex const & index, ScrollHint hint);
		
		void performTimeSynchronization (int sync_group, unsigned long long time, void * source);
		void performFrameSynchronization (int sync_group, unsigned long long frame, void * source);

		void onClearAllDataButton ();
		void onNextToView ();
		void turnOffAutoScroll ();
		void onAutoScrollHotkey ();
		void onQSearch (QString const & text);
		void onQSearchEditingFinished ();
		void setLastSearchIntoCombobox (QString const & txt);
		void onFindAllButton ();
		void onQFilterLineEditFinished ();
		void appendToSearchHistory (QString const & str);
		void updateSearchHistory ();
		void onEditFindNext ();
		void onEditFindPrev ();
		void onDumpFilters ();
		void applyConfig ();
		void exportStorageToCSV (QString const & filename);

		QString onCopyToClipboard ();
		void findTableIndexInFilters (QModelIndex const & src_idx, bool scroll_to_item, bool expand);

	signals:
		//void requestTimeSynchronization (int sync_group, unsigned long long time, void * source);
		//void requestFrameSynchronization (int sync_group, unsigned long long frame, void * source);

	protected:
		QString m_fname;
		Connection * m_connection;
		QWidget * m_tab;

		FilterState m_filter_state;

		// mutable state
		TagConfig m_tagconfig;
		QMap<tlv::tag_t, int> m_tags2columns;

		QString m_last_search;
		int m_current_tag;
		int m_current_selection;
		unsigned long long m_time_ref_value;

		QString m_curr_preset;
		//QString m_csv_separator;
		//QTextStream * m_file_csv_stream;
};
