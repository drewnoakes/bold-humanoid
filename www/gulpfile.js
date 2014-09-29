/// <reference path="scripts/libs/gulp.d.ts" />

// TODO investigate gulp-type for incremental compiles https://www.npmjs.org/package/gulp-type/
// TODO investigate using browserify or webpack and AMD modules
// TODO investigate keeping sourcemaps through pipeline
//      - code: https://github.com/gulpjs/gulp/blob/master/docs/recipes/browserify-uglify-sourcemap.md
//      - styles: https://www.npmjs.org/package/gulp-autoprefixer
// TODO investigate running tests using gulp

var gulp = require('gulp');

var sass = require('gulp-sass');
var concat = require('gulp-concat');
var uglify = require('gulp-uglify');
var rename = require('gulp-rename');
var autoprefixer = require('gulp-autoprefixer');
var minifycss = require('gulp-minify-css');
var fs = require('fs');
var header = require('gulp-header');
var typescript = require('gulp-tsc');
var amdOptimize = require('amd-optimize');
var inject = require('gulp-inject');
var imagemin = require('gulp-imagemin');
var pngcrush = require('imagemin-pngcrush');

var outFolder = 'dist';

// Transpiles SASS styles, runs autoprefixer and saves as a .css file
gulp.task('styles', function ()
{
    return gulp.src('styles/*.scss')
        .pipe(sass())
        .pipe(autoprefixer('last 2 versions', 'safari 5', 'ie 8', 'ie 9', 'opera 12.1'))
        .pipe(gulp.dest('styles'))
});

// Transpiles TypeScript source code
gulp.task('tsc', function ()
{
    return gulp.src('scripts/app/**/*.ts')
        .pipe(typescript({module:'amd'}))
        .pipe(gulp.dest('scripts/app/'))
});

gulp.task('bundle-styles', ['styles'], function ()
{
    var styles = [
        'styles/round-table.css',
        'styles/joint.css'
    ];

    return gulp.src(styles)
        .pipe(concat('styles.css'))
        .pipe(minifycss())
        .pipe(header(fs.readFileSync('LICENSE')))
        .pipe(gulp.dest(outFolder));
});

gulp.task('bundle-source', ['tsc'], function ()
{
    // TODO maybe use CommonJS and ditch RequireJS
    // TODO sourcemap support
    return gulp.src('scripts/app/**/*.js')
        .pipe(amdOptimize('main', {baseUrl: 'scripts/app'}))
        .pipe(concat('main.js'))
        .pipe(uglify())
        .pipe(header(fs.readFileSync('LICENSE')))
        .pipe(gulp.dest(outFolder));
});

gulp.task('bundle-libs', function ()
{
    var libs = [
        'scripts/libs/three.js',
        'scripts/libs/smoothie.js',
        'scripts/libs/lodash.js',
        'scripts/libs/jquery-2.0.3.js',
        'scripts/libs/jquery-ui-1.10.2.custom.js',
        'scripts/libs/hammer-1.1.3.js',
        'scripts/libs/handlebars.js',
        'scripts/libs/d3-4.3.8.js',
        'scripts/libs/joint.js',
        'scripts/libs/joint.layout.DirectedGraph.js'
    ];

    // TODO delete minified versions of libraries from disk

    return gulp.src(libs)
        .pipe(concat('libs.js'))
        .pipe(uglify())
        .pipe(gulp.dest(outFolder));
});

gulp.task('bundle-images', function ()
{
    return gulp.src('./images/*', {base: './'})
        .pipe(imagemin({
            progressive: true,
            svgoPlugins: [{removeViewBox: false}],
            use: [pngcrush()]
        }))
        .pipe(gulp.dest(outFolder));
});

// Produce a distributable version of the site as a self-contained bundle
gulp.task('dist', ['bundle-images', 'bundle-styles', 'bundle-source', 'bundle-libs'], function ()
{
    var sources = [
        'dist/libs.js',
        'dist/main.js',
        'dist/styles.css'];

    return gulp.src('index.html')
        .pipe(rename('dist/index.html'))
        .pipe(inject(gulp.src(sources), {read: false, relative: true}))
        //.pipe(minifyhtml())
        .pipe(gulp.dest('./'));
});

gulp.task('watch', function ()
{
    gulp.watch('scripts/app/**/*.ts', ['tsc']);
    gulp.watch('styles/*.scss', ['styles']);
});

gulp.task('default', ['styles', 'tsc']);
