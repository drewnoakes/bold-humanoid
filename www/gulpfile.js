/// <reference path="scripts/libs/gulp.d.ts" />

// TODO investigate gulp-type for incremental compiles https://www.npmjs.org/package/gulp-type/
// TODO investigate keeping sourcemaps through pipeline
//      - code: https://github.com/gulpjs/gulp/blob/master/docs/recipes/browserify-uglify-sourcemap.md
//      - code: http://stackoverflow.com/questions/23453160/keep-original-typescript-source-maps-after-using-browserify
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
var inject = require('gulp-inject');
var imagemin = require('gulp-imagemin');
var pngcrush = require('imagemin-pngcrush');
var jsonminify = require('gulp-jsonminify');
var browserify = require('browserify');
var source = require('vinyl-source-stream');
var streamify = require('gulp-streamify');
var combinemediaqueries = require('gulp-combine-media-queries');
var del = require('del');
var plumber = require('gulp-plumber');

var distFolder = 'dist';
var buildFolder = 'build';

// Transpiles SASS styles, runs autoprefixer and saves as a .css file
gulp.task('compile-sass', function ()
{
    return gulp.src('styles/*.scss')
        .pipe(plumber())
        .pipe(sass())
        .pipe(autoprefixer('last 2 versions', 'safari 5', 'ie 8', 'ie 9', 'opera 12.1'))
        .pipe(gulp.dest(buildFolder + '/styles'))
});

// Transpiles TypeScript source code
function compileTypeScript(moduleType, outFolder)
{
    return function()
    {
        return gulp.src('scripts/**/*.ts')
            .pipe(plumber())
            .pipe(typescript({module: moduleType}))
            .pipe(gulp.dest(outFolder))
    };
}

gulp.task('compile-commonjs', compileTypeScript('commonjs', buildFolder + '/commonjs/'));
gulp.task('compile-amd',      compileTypeScript('amd',      buildFolder + '/amd/'));

gulp.task('clean-dist', function(cb)
{
    del([distFolder], cb);
});

gulp.task('bundle-styles', ['clean-dist', 'compile-sass'], function ()
{
    var styles = [
        'build/styles/round-table.css',
        'styles/joint.css'
    ];

    return gulp.src(styles)
        .pipe(concat('styles.css'))
        .pipe(combinemediaqueries())
        .pipe(minifycss({keepSpecialComments:0}))
        .pipe(header(fs.readFileSync('LICENSE')))
        .pipe(gulp.dest(distFolder));
});

gulp.task('bundle-source', ['clean-dist', 'compile-commonjs'], function ()
{
    // TODO sourcemap support

    return browserify('main.js', {
            basedir: './' + buildFolder + '/commonjs/',
            paths: ['./' + buildFolder + '/commonjs/'],
            builtins: {constants: null, util: null}
        })
        .bundle()
        .pipe(source('main.js'))
        .pipe(streamify(uglify()))
        .pipe(header(fs.readFileSync('LICENSE')))
        .pipe(gulp.dest(distFolder));
});

gulp.task('bundle-libs', ['clean-dist'], function ()
{
    var libs = [
        'libs/three.js',
        'libs/smoothie.js',
        'libs/lodash.js',
        'libs/jquery-2.0.3.js',
        'libs/jquery-ui-1.10.2.custom.js',
        'libs/hammer-1.1.3.js',
        'libs/handlebars.js',
        'libs/d3-4.3.8.js',
        'libs/joint.js',
        'libs/joint.layout.DirectedGraph.js'
    ];

    return gulp.src(libs)
        .pipe(concat('libs.js'))
        .pipe(uglify())
        .pipe(gulp.dest(distFolder));
});

gulp.task('bundle-images', ['clean-dist'], function ()
{
    return gulp.src('./resources/images/*', {base: './'})
        .pipe(imagemin({
            progressive: true,
            svgoPlugins: [{removeViewBox: false}],
            use: [pngcrush()]
        }))
        .pipe(gulp.dest(distFolder));
});

gulp.task('bundle-fonts', ['clean-dist'], function ()
{
    return gulp.src('./resources/fonts/*', {base: './'})
        .pipe(gulp.dest(distFolder));
});

gulp.task('bundle-models', ['clean-dist'], function ()
{
    return gulp.src(['/resources/darwin.json'], {base: './'})
        .pipe(jsonminify())
        .pipe(gulp.dest(distFolder));
});

var allBundles = [
    'bundle-source',
    'bundle-images',
    'bundle-styles',
    'bundle-libs',
    'bundle-fonts',
    'bundle-models'
];

// Produce a distributable version of the site as a self-contained bundle
gulp.task('dist', allBundles, function ()
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
    gulp.watch('scripts/**/*.ts', ['compile-amd']);
    gulp.watch('styles/*.scss',   ['compile-sass']);
});

gulp.task('compile', ['compile-sass', 'compile-amd']);

gulp.task('default', ['compile']);
