/**
 * Copyright (C) 2011 Mojmir Svoboda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/
#pragma once
#include <vector>
#include <3rd/assocvector.h>
#include "baseproxymodel.h"
#include <filterproxymodel.h>

class SparseProxyModel : public BaseProxyModel
{
	Q_OBJECT

public:
	explicit SparseProxyModel (QObject * parent);

protected:

	virtual bool filterAcceptsColumn (int sourceColumn, QModelIndex const & source_parent) const;
	virtual bool filterAcceptsRow (int sourceRow, QModelIndex const & sourceParent) const;
};

class SparseFilterProxyModel : public FilterProxyModel
{
	Q_OBJECT

public:
	explicit SparseFilterProxyModel (QObject * parent);

protected:

	virtual bool filterAcceptsColumn (int sourceColumn, QModelIndex const & source_parent) const;
	virtual bool filterAcceptsRow (int sourceRow, QModelIndex const & sourceParent) const;
};



