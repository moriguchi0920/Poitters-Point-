# BaseProject2026
## 概要
### BaseProjectとは？
DXライブラリを使用して3Dゲーム開発をするためのプロジェクトです。<br />

## 使用環境
- OS: Windows10, Windows11
- IDE: VisualStudio2022 / VisualStudio2026

## 使い方
### プロジェクトの起動方法:

「＠open.cmd」ファイルをダブルクリックしてください。<br />
自動的にVisualStudio2022でプロジェクトが開きます。<br />

### プロジェクトの終了方法

「＠cleanup.cmd」をダブルクリックしてください。<br />
自動的に不要なファイルが削除されます。<br />

### コードの評価

「＠code_metrics.cmd」をダブルクリックしてください。<br />
自動的に「src」フォルダ内のソースファイル全てが「cccc」によって評価されます。<br />

### Doxygenの作成／修正

「＠doxygen.cmd」をダブルクリックしてください。<br />
自動的に「src」フォルダ内のソースファイル全てを解析してドキュメントを作成します。<br />
<b>注意事項: Doxygen形式でのコメントの書き方をしていないとドキュメントに反映されません</b>

### コード整形

「src/.clang-format-reference」を「.clang-format」にリネームすることで自分好みのソースコードに整形が可能です。<br />
また「＠open.cmd」を使用することで自動的にこのファイルを読み込み、コードを自分好みに整形してくれます。<br />
<b>注意事項: 「＠code_format.cmd」を使うと元のコード整形に戻ります</b>
