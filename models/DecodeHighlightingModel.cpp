#include "DecodeHighlightingModel.hpp"

#include <QString>
#include <QVariant>
#include <QList>
#include <QBrush>
#include <QColor>
 #include <QFont>
#include <QMap>
#include <QVector>
#include <QDataStream>
#include <QMetaType>
#include <QDebug>

#include "pimpl_impl.hpp"

#include "moc_DecodeHighlightingModel.cpp"

class DecodeHighlightingModel::impl final
{
public:
  explicit impl ()
    : data_ {defaults_}
  {
  }

  HighlightItems static const defaults_;
  HighlightItems data_;
  QFont font_;
};

QList<DecodeHighlightingModel::HighlightInfo> const DecodeHighlightingModel::impl::defaults_ = {   //avt 4/23/25
  {Highlight::MyCall, true, {{Qt::white}}, {{Qt::red}}}
  , {Highlight::Continent, false, {}, {{0xff, 0x00, 0x63}}}
  , {Highlight::ContinentBand, false, {}, {{0xff, 0x99, 0xc2}}}
  , {Highlight::CQZone, false, {}, {{0xff, 0xbf, 0x00}}}
  , {Highlight::CQZoneBand, false, {}, {{0xff, 0xe4, 0x99}}}
  , {Highlight::ITUZone, false, {}, {{0xa6, 0xff, 0x00}}}
  , {Highlight::ITUZoneBand, false, {}, {{0xdd, 0xff, 0x99}}}
  , {Highlight::DXCC, true, {}, {{0xff, 0x00, 0xff}}}
  , {Highlight::DXCCBand, true, {}, {{0xff, 0xaa, 0xff}}}
  , {Highlight::Grid, false, {}, {{0xff, 0x80, 0x00}}}
  , {Highlight::GridBand, false, {}, {{0xff, 0xcc, 0x99}}}
  , {Highlight::Call, true, {}, {{0x00, 0xff, 0xff}}}
  , {Highlight::CallBand, true, {}, {{0x99, 0xff, 0xff}}}
  , {Highlight::LotW, false, {{0x99, 0x00, 0x00}}, {}}
  , {Highlight::CQ, true, {}, {}}
  , {Highlight::Tx, true, {}, {{Qt::yellow}}}
};

bool operator == (DecodeHighlightingModel::HighlightInfo const& lhs, DecodeHighlightingModel::HighlightInfo const& rhs)
{
  return lhs.type_ == rhs.type_
    && lhs.enabled_ == rhs.enabled_
    && lhs.foreground_ == rhs.foreground_
    && lhs.background_ == rhs.background_;
}

QDataStream& operator << (QDataStream& os, DecodeHighlightingModel::HighlightInfo const& item)
{
  return os << item.type_
            << item.enabled_
            << item.foreground_
            << item.background_;
}

QDataStream& operator >> (QDataStream& is, DecodeHighlightingModel::HighlightInfo& item)
{
  return is >> item.type_
           >> item.enabled_
           >> item.foreground_
           >> item.background_;
}

#if !defined (QT_NO_DEBUG_STREAM)
QDebug operator << (QDebug debug, DecodeHighlightingModel::HighlightInfo const& item)
{
  QDebugStateSaver save {debug};
  debug.nospace () << "HighlightInfo("
                   << item.type_ << ", "
                   << item.enabled_ << ", "
                   << item.foreground_ << ", "
                   << item.background_ << ')';
  return debug;
}
#endif

ENUM_QDATASTREAM_OPS_IMPL (DecodeHighlightingModel, Highlight);
ENUM_CONVERSION_OPS_IMPL (DecodeHighlightingModel, Highlight);

DecodeHighlightingModel::DecodeHighlightingModel (QObject * parent)
  : QAbstractListModel {parent}
{
}

DecodeHighlightingModel::~DecodeHighlightingModel ()
{
}

QString DecodeHighlightingModel::highlight_name (Highlight h)
{
  switch (h)
    {
    case Highlight::CQ: return "CQ in message";
    case Highlight::MyCall: return "My Call in message";
    case Highlight::Tx: return "Transmitted message";
    case Highlight::DXCC: return "New DXCC";
    case Highlight::DXCCBand: return "New DXCC on Band";
    case Highlight::Grid: return "New Grid";
    case Highlight::GridBand: return "New Grid on Band";
    case Highlight::Call: return "New Call";
    case Highlight::CallBand: return "New Call on Band";
    case Highlight::Continent: return "New Continent";
    case Highlight::ContinentBand: return "New Continent on Band";
    case Highlight::CQZone: return "New CQ Zone";
    case Highlight::CQZoneBand: return "New CQ Zone on Band";
    case Highlight::ITUZone: return "New ITU Zone";
    case Highlight::ITUZoneBand: return "New ITU Zone on Band";
    case Highlight::LotW: return "LotW User";
    }
  return "Unknown";
}

auto DecodeHighlightingModel::default_items () -> HighlightItems const&
{
  return impl::defaults_;
}

auto DecodeHighlightingModel::items () const -> HighlightItems const&
{
  return m_->data_;
}

void DecodeHighlightingModel::items (HighlightItems const& items)
{
  m_->data_ = items;
  QVector<int> roles;
  roles << Qt::CheckStateRole << Qt::ForegroundRole << Qt::BackgroundRole;
  Q_EMIT dataChanged (index (0, 0), index (rowCount () - 1, 0), roles);
}

void DecodeHighlightingModel::set_font (QFont const& font)
{
  m_->font_ = font;
}

int DecodeHighlightingModel::rowCount (const QModelIndex& parent) const
{
  return parent.isValid () ? 0 : m_->data_.size ();
}

QVariant DecodeHighlightingModel::data (const QModelIndex& index, int role) const
{
  QVariant result;
  if (index.isValid () && index.row () < rowCount ())
    {
      auto const& item = m_->data_[index.row ()];
      switch (role)
        {
        case Qt::CheckStateRole:
          result = item.enabled_ ? Qt::Checked : Qt::Unchecked;
          break;
        case Qt::DisplayRole:
          return QString {"%1 [f/g:%2, b/g:%3]"}
             .arg (highlight_name (item.type_))
             .arg (item.foreground_.style () != Qt::NoBrush ? QString {"#%1"}.arg (item.foreground_.color ().rgb () & 0xffffff, 6, 16, QChar {'0'}) : QString {"unset"})
             .arg (item.background_.style () != Qt::NoBrush ? QString {"#%1"}.arg (item.background_.color ().rgb () & 0xffffff, 6, 16, QChar {'0'}) : QString {"unset"});
          break;
        case Qt::ForegroundRole:
          if (Qt::NoBrush != item.foreground_.style ())
            {
              result = item.foreground_;
            }
          break;
        case Qt::BackgroundRole:
          if (Qt::NoBrush != item.background_.style ())
            {
              result = item.background_;
            }
          break;
        case Qt::FontRole:
          result = m_->font_;
          break;
        case TypeRole:
          result = static_cast<int> (item.type_);
          break;
        case EnabledDefaultRole:
          for (auto const& default_item : impl::defaults_)
            {
              if (default_item.type_ == item.type_)
                {
                  result = default_item.enabled_ ? Qt::Checked : Qt::Unchecked;
                }
            }
          break;
        case ForegroundDefaultRole:
          for (auto const& default_item : impl::defaults_)
            {
              if (default_item.type_ == item.type_)
                {
                  result = default_item.foreground_;
                }
            }
          break;
        case BackgroundDefaultRole:
          for (auto const& default_item : impl::defaults_)
            {
              if (default_item.type_ == item.type_)
                {
                  result = default_item.background_;
                }
            }
          break;
        }
    }
  return result;
}

// Override  QAbstractItemModel::itemData()  as  it  is  used  by  the
// default mime encode  routine used in drag'n'drop  operations and we
// want to transport  the type role, this is because  the display role
// is derived from the type role.
QMap<int, QVariant> DecodeHighlightingModel::itemData (QModelIndex const& index) const
{
  auto roles = QAbstractListModel::itemData (index);
  QVariant variantData = data (index, TypeRole);
  if (variantData.isValid ())
    {
      roles.insert (TypeRole, variantData);
    }
  return roles;
}

QVariant DecodeHighlightingModel::headerData (int /*section*/, Qt::Orientation orientation, int role) const
{
  QVariant header;
  if (Qt::DisplayRole == role && Qt::Horizontal == orientation)
    {
      header = tr ("Highlight Type");
    }
  return header;
}

Qt::ItemFlags DecodeHighlightingModel::flags (QModelIndex const& index) const
{
  auto flags = QAbstractListModel::flags (index) | Qt::ItemIsDragEnabled;
  if (index.isValid ())
    {
      flags |= Qt::ItemIsUserCheckable;
    }
  else
    {
      flags |= Qt::ItemIsDropEnabled;
    }
  return flags;
}

bool DecodeHighlightingModel::setData (QModelIndex const& index, QVariant const& value, int role)
{
  bool ok {false};
  if (index.isValid () && index.row () < rowCount ())
    {
      auto& item = m_->data_[index.row ()];
      QVector<int> roles;
      roles << role;
      switch (role)
        {
        case Qt::DisplayRole:
        case Qt::FontRole:
          ok = true;
          break;
        case Qt::CheckStateRole:
          if (item.enabled_ != (Qt::Checked == value))
            {
              item.enabled_ = Qt::Checked == value;
              Q_EMIT dataChanged (index, index, roles);
            }
          ok = true;
          break;
        case Qt::ForegroundRole:
          if (item.foreground_ != value.value<QBrush> ())
            {
              item.foreground_ = value.value<QBrush> ();
              Q_EMIT dataChanged (index, index, roles);
            }
          ok = true;
          break;
        case Qt::BackgroundRole:
          if (item.background_ != value.value<QBrush> ())
            {
              item.background_ = value.value<QBrush> ();
              Q_EMIT dataChanged (index, index, roles);
            }
          ok = true;
          break;
        case TypeRole:
          if (item.type_ != static_cast<Highlight> (value.toInt ()))
            {
              item.type_ = static_cast<Highlight> (value.toInt ());
              roles << Qt::DisplayRole;
              Q_EMIT dataChanged (index, index, roles);
            }
          ok = true;
          break;
        }
    }
  return ok;
}

Qt::DropActions DecodeHighlightingModel::supportedDropActions () const
{
  return Qt::MoveAction;
}

bool DecodeHighlightingModel::insertRows (int row, int count, QModelIndex const& parent)
{
  beginInsertRows (parent, row, row + count - 1);
  for (int index = 0; index < count; ++index)
    {
      m_->data_.insert (row, HighlightInfo {Highlight::CQ, false, {}, {}});
    }
  endInsertRows ();
  return true;
}

bool DecodeHighlightingModel::removeRows (int row, int count, QModelIndex const& parent)
{
  beginRemoveRows (parent, row, row + count - 1);
  for (int index = 0; index < count; ++index)
    {
      m_->data_.removeAt (row);
    }
  endRemoveRows ();
  return true;
}
