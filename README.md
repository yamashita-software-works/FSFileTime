# FSFileTime

FSFileTimeは、Windowsシステム上でファイルやディレクトリの日時を表示・設定するコンソールツールです。

Windows 7以降で動作を確認しています。

## 基本構文

    fsfiletime command [options] filename

## 表示(QUERY)コマンド

ファイルやディレクトリの日時を表示します。

    fsfiletime query [options] filename

`options` 表示オプションです。主なオプションは以下の通りです。

    -t:<type>　表示する日付の種類と順番を指定します。

    <type>
        w  更新日時
        c  作成日時
        a  最終アクセス日時
        h  属性変更日時
        デフォルトは "-t:wcah" になります。

    -l 結果を1行で表示します。
    -m ミリ秒まで表示します。

    -fd:<format-date> 表示する日付のフォーマットを指定します。 デフォルトはシステム設定です。
                      詳細はWin32 APIのGetDateFormat/GetDateFormatExの書式してください。

    -ft:<format-time> 表示する時刻のフォーマットを指定します。デフォルトは"HH:mm:ss"です。
                      詳細はWin32 APIのGetTimeFormat/GetTimeFormatExの書式してください。

日付の書式は[こちら](https://docs.microsoft.com/en-us/windows/win32/intl/day--month--year--and-era-format-pictures)を、時刻の書式は[こちら](https://docs.microsoft.com/en-us/windows/win32/intl/hour--minute--and-second-format-pictures)を参照してください。

`filename` 日時を表示するファイルやディレクトリ名。パスを指定できます。  
名前にワイルドカードが含まれる場合、指定されたディレクトリ内の一致するディレクトリ，ファイルすべてを表示します。その際サブディレクトリの階層に含まれるディレクトリ，ファイルは表示されません。

#### 操作例

    fsfiletime query myfile.txt
    fsfiletime query -t:w c:\windows\notepad.exe
    fsfiletime query c:\windows\*.*
    fsfiletime query -fd:"yyyy-MM-dd(ddd)" -ft:"tt hh:mm:ss" c:\foo\*.*

## 設定(SET)コマンド

ファイルやディレクトリの日時を設定します。

    fsfiletime set {date|time|bin} [options] filename

`date` 日付を指定します。

    -dw:<date> 更新日付を設定
    -dc:<date> 作成日付を設定
    -da:<date> 最終アクセス日付を設定
    -dh:<date> 属性変更日付を設定
       または
    -dz:<date> すべての種類の日付を設定

    <date>  yyyy-mm-dd (西暦-月-日)の形式で指定します。
            日付の変わりに '@now' を指定すると現在の日付を使用します。
            ※日付の指定順は国や地域,文化によって異なりますが、現状では上記の指定順のみ対応しています。
            例) -dw:"2019-01-02"
                -dw:@now

`time` 時刻を指定します。

    -tw:<time> 更新時刻を設定
    -tc:<time> 作成時刻を設定
    -ta:<time> 最終アクセス時刻を設定
    -th:<time> 属性変更時刻を設定
       または
    -tz:<time> すべての種類の時刻を設定

    <time>  hh:mm:ss (時:分:秒)の形式で指定します。
            例) -tw:"12:34:56"
                -tw:@now

`bin` 日時を64bitの値（絶対システム時間）で指定します。

    -bw:<bin> 更新日時の絶対システム時間を設定
    -bc:<bin> 作成日時の絶対システム時間を設定
    -ba:<bin> 最終アクセス日時の絶対システム時間を設定
    -bh:<bin> 属性変更日時の絶対システム時間を設定
       または
    -bz:<bin> すべての種類の絶対システム時間を設定

    <bin>  数字のみの場合10進数、'0x'プリフィックスが付いた場合16進数として扱います。
           例) -bz:0x01D4DF2D96F8D800

`options` 表示オプションです。主なオプションは以下の通りです。

    -y 設定時の確認を省略する。
    -test 実際には設定せず、対象となるファイル名を表示する。
    -u, -utc 日時をローカルではなくUTCとして扱う。
    -fat, -dos 日時をMS-DOS(FAT16)時間として設定する。指定された時間の秒は偶数になります。

`filename` 日時を表示するファイルやディレクトリ名。パスを指定できます。  
名前にワイルドカードが含まれる場合、指定されたディレクトリ内の一致するディレクトリ，ファイルすべてに設定します。その際サブディレクトリの階層に含まれるディレクトリ，ファイルには設定しません。

#### 操作例

    fsfiletime set -dw:2019-01-01 -tw:0:0:0 c:\foo\bar.txt
    fsfiletime set -tw:12:34:56 c:\foo\*
    fsfiletime set -dw:@now -tw:@now myfile.txt
    fsfiletime set -bz:0x01D4DF2D96F8D800 c:\foo\bar.*

## TimeToBin コマンド

    fsfiletime timetobin date [time] [options]

日時データを絶対システム時間で表示します。

`date` 日付を指定します。

    date  yyyy-mm-dd (西暦-月-日)の形式で指定します。
          日付の変わりに '@now' を指定すると現在の日付を使用します。

`time` 時刻を指定します。

    time  hh:mm:ss (時:分:秒)の形式で指定します。
          時刻の変わりに '@now' を指定すると現在の時刻を使用します。
          時刻を省略した場合、0:0:0として扱います。

`options` 表示オプションです。

    -u, -utc    指定された日時をUTCとして扱います。

## BinToTime コマンド

絶対システム時間を日時書式で表示します。

    fsfiletime bintotime value [options]

`value` 絶対時間データを指定します。

    value     数字のみの場合 10進数として扱います。
              '0x' プリフックスが付いた場合16進数として扱います。

`options` 表示または指定データへのオプションです。

    -dos      指定された値を1980年からの絶対秒数として扱います。
    -unix     指定された値を1970年からの絶対秒数として扱います。
    -u, -utc  日時をUTCとして表示します。
    -m        ミリ秒まで表示します。

## Help コマンド

すべてのコマンドとオプションを表示します。

    fsfiletime help

## Build

ソースからビルドするには　Windows Driver Kit Version 7.1.0 (WDK)が必要です。

https://www.microsoft.com/en-us/download/details.aspx?id=11800

インストールした後、スタートメニューの

Windows Driver Kits>WDK 7600.16385.1>Build Environments>Windows 7

から

64ビット版をビルドする場合は、`x64 Free Build Environment`

32ビット版をビルドする場合は、 `x86 Free Build Environment`

を開き、clone先またはソースの展開先ディレクトリへ移動して`build`コマンドを実行します。

<br>

Visal Studio 2010でもビルドできますが、プロジェクトのプロパティで先にインストールしたWDKディレクトリへの設定が必要になります。

例えばWDKを`C:\WinDDK\7600.16385.1`にインストールした場合、以下の様になります。

- [VC++ディレクトリ][インクルードディレクトリ]  
`C:\WinDDK\7600.16385.1\inc\ddk;C:\WinDDK\7600.16385.1\inc\api;C:\WinDDK\7600.16385.1\inc\crt;$(IncludePath)`

- [リンカー全般] [追加のライブラリディレクトリ]  
構成がx64の場合  
`C:\WinDDK\7600.16385.1\lib\win7\amd64`  
構成がWin32 (x86)の場合  
`C:\WinDDK\7600.16385.1\lib\win7\i386`

## License

Copyright (C) YAMASHITA Katsuhiro. All rights reserved.

Licensed under the [MIT](LICENSE) License.
